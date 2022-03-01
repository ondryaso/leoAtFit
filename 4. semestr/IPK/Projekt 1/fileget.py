#!/usr/local/bin/python3

# IPK – Project #1 – Trivial distributed filesystem client
# Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)
# Date:   2021-03-13
# Error return codes:
#  - 1: invalid argument values
#  - 2: missing arguments or NSP request error
#  - 3: the requested FSP hostname does not exist
#  - 4: FSP request error (e.g. timeout, network error, malformed response)
#  - 5: the requested file does not exist
#  - 6: FSP returned an error
#  - 7: error while saving a file

import argparse
import ipaddress
import os
import re
import sys
from enum import Enum
from socket import socket, AF_INET, SOCK_DGRAM, SOCK_STREAM
from typing import Optional, Union, Tuple, Dict


def print_err(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


class FSPResponseCode(Enum):
    SUCCESS = 'Success'
    BAD_REQ = 'Bad Request'
    NOT_FOUND = 'Not Found'
    ERROR = 'Server Error'

    @classmethod
    def get(cls, name: str):
        name = name.strip().lower()
        for a in cls:
            if a.value.lower() == name:
                return a

        return None


class FSPClient:
    def __init__(self, address: str, port: int, hostname: str):
        self.address = address
        self.port = port
        self.hostname = hostname

    @staticmethod
    def __parse_header(header: str) -> Tuple[FSPResponseCode, int]:
        """Parses a FSP/1.0 header and returns a tuple of the response code and the length of data.
        Unknown headers are ignored. The Length header is required.

        :param header: A string containing the complete header data. Each line must end with CRLF.
        :return: A tuple of the parsed out response code and the length of data.
        :raises InvalidFSPResponseError: when the header is malformed or doesn't contain a valid Length header.
        """
        lines = header.split('\r\n')
        if len(lines) < 2:
            raise InvalidFSPResponseError('Not enough lines in header')
        code = lines[0].partition(' ')
        if len(code[2]) == 0:
            raise InvalidFSPResponseError('Invalid response status code')

        resp_code = FSPResponseCode.get(code[2])

        for line in lines:
            line_parts = line.partition(':')
            if line_parts[0] == 'Length':
                try:
                    length = int(line_parts[2].strip())
                    return resp_code, length
                except ValueError as e:
                    raise InvalidFSPResponseError('Invalid value of Length header') from e
        else:
            raise InvalidFSPResponseError('Length header not found')

    def __make_get_header(self, path: str):
        return f"GET {path} FSP/1.0\r\nHostname: {self.hostname}\r\nAgent: xondry02\r\n\r\n"

    def __receive(self, client_socket: socket) -> Tuple[FSPResponseCode, int, bytearray]:
        """Receives data from an opened client socket, parses it and returns a tuple with the received data.

        :param client_socket: An opened socket to receive data from.
        :return: A tuple of response code, length of the
        received data and a bytearray object with the received data.
        :raises InvalidFSPResponseError: when the received message does not contain a valid FSP/1.0 response
        or when the socket is closed before a valid message can be read.
        """
        chunks = bytearray()
        # 0 - waiting for protocol
        # 1 - protocol ok, waiting for delimiter/header
        # 2 - header ok, waiting for data
        # 3 - all data read
        state = 0

        delimiter = b'\r\n\r\n'
        header = None
        data_left = None
        index = 0

        while True:
            chunk = client_socket.recv(64)
            chunks.extend(chunk)

            if state == 0:
                if chunk == b'':
                    raise InvalidFSPResponseError('Socket broken')
                if len(chunks) < 7:
                    continue  # wait for more data
                if chunks[0:7] != b'FSP/1.0':
                    raise InvalidFSPResponseError('Invalid protocol version')
                state = 1
            elif state == 1:
                index = chunks.index(delimiter)
                if index == -1:
                    if chunk == b'':
                        raise InvalidFSPResponseError('Socket broken')
                    continue  # wait for more data

                header = self.__parse_header(chunks[0:index].decode('ascii'))
                data_left = header[1] - (len(chunks) - index - 4)
                state = 2 if data_left > 0 else 3
            elif state == 2:
                data_left -= len(chunk)
                if data_left <= 0:
                    state = 3
                elif chunk == b'':
                    raise InvalidFSPResponseError('Socket broken')
            elif state == 3:
                client_socket.close()
                return header[0], header[1], chunks[(index + 4):]

    def __get_all(self, path: str) -> Dict[str, Optional[Tuple[FSPResponseCode, int, bytearray]]]:
        """Fetches the index pseudo-file from the server, then fetches all files from the server,
        according to the index file's contents. Catches any exception that occurs while transferring a file,
        printing its message to stderr and marking error by saving None in the returned dictionary.

        :param path: The remote path to fetch. The path must end with an asterisk.
        :return: A dictionary where keys are the fetched paths and values are the corresponding data tuples
        (as returned by the get() method) or None if an error occurred while transferring the file.
        """
        if path.count('*') > 1 or path[-1] != '*':
            raise InvalidTargetAddressError(path)
        path = path[:-1]

        index_file = self.get('index')
        if index_file[0] != FSPResponseCode.SUCCESS:
            raise UnsuccessfulFSPResponse(index_file[0], 'Cannot fetch the index file')

        index = index_file[2].decode('utf-8')
        index_lines = filter(lambda l: l.startswith(path), index.splitlines(False))
        ret = {}
        for line in index_lines:
            try:
                resp = self.get(line)
                ret[line] = resp
            except Exception as e:
                print_err(f"Error fetching file '{line}': {str(e)}")
                ret[line] = None

        return ret

    def get(self, path: str) -> \
            Union[Tuple[FSPResponseCode, int, bytearray], Dict[str, Optional[Tuple[FSPResponseCode, int, bytearray]]]]:
        """
        Gets a file on the specified path from the remote server. If a single file is requested, a tuple of response
        code, data length and the binary file contents is returned; if multiple files are requested (using a path
        ending with an asterisk), a dictionary where keys are the fetched paths and values are the aforementioned tuples
        or None objects if an error occurred while fetching the corresponding file.

        :param path: The path to fetch.
        :return: A tuple with the downloaded data; or a dictionary of these tuples if multiple files were requested.
        """
        if '*' in path:
            return self.__get_all(path)

        with socket(AF_INET, SOCK_STREAM) as client_socket:
            client_socket.settimeout(10)
            client_socket.connect((self.address, self.port))

            header = self.__make_get_header(path)
            client_socket.sendall(header.encode('utf-8'))

            resp = self.__receive(client_socket)
            return resp


class NSPClient:
    def __init__(self, address: str, port: int):
        self.address = address
        self.port = port

    def resolve(self, name: str, timeout: int = 5, max_tries: int = 3) -> Optional[Tuple[str, int]]:
        """Performs a NSP query and returns a tuple of the resolved IP and port.
        Returns None if the server is not found.

        :returns: A tuple of IP address and port or None if the requested name has not been found.
        :raises InvalidNSPRequestError: when an error different to Not Found is returned from the server.
        :raises InvalidNSPResponseError: when the returned message is malformed.
        :raises OSError: when a socket communication error (including timeout) occurs.
        """

        with socket(AF_INET, SOCK_DGRAM) as client_socket:
            client_socket.settimeout(timeout)

            addr = (self.address, self.port)
            data = ('WHEREIS ' + name).encode('ascii')

            for i in range(max_tries):
                try:
                    client_socket.sendto(data, addr)
                    # the longest expected response 'OK 255.255.255.255:65535' is 24 bytes long
                    received = client_socket.recvfrom(64)
                    received_str = received[0].decode('ascii')
                    return self.parse_response(received_str)
                except OSError as e:
                    if i == max_tries - 1:
                        raise e
                    else:
                        print_err(f"No response from the nameserver, retrying ({i + 2}/{max_tries})")

    @staticmethod
    def parse_response(resp: str) -> Optional[Tuple[str, int]]:
        if resp.find('OK ') == 0:
            resp_parts = resp[3:].partition(':')
            if resp_parts[2] == '':
                raise InvalidNSPResponseError(resp)

            try:
                return resp_parts[0], int(resp_parts[2])
            except ValueError as e:
                raise InvalidNSPResponseError(resp) from e
        elif resp.find('ERR ') == 0:
            if resp[4:].strip() == 'Not Found':
                return None

            raise InvalidNSPRequestError(resp)
        else:
            raise InvalidNSPResponseError(resp)


class TargetAddressMatchingError(Exception):
    pass


class InvalidTargetAddressError(Exception):
    pass


class InvalidNSPRequestError(Exception):
    pass


class InvalidNSPResponseError(Exception):
    pass


class InvalidFSPResponseError(Exception):
    pass


class UnsuccessfulFSPResponse(Exception):
    def __init__(self, code: FSPResponseCode, *args):
        super().__init__(code, *args)
        self.code = code


name_regex = re.compile('^fsp://([a-zA-Z0-9\\-_.]+)/(.+)$', re.IGNORECASE)


def parse_target_address(name: str) -> Tuple[str, str]:
    matches = name_regex.fullmatch(name)
    if matches is None:
        raise InvalidTargetAddressError()

    if len(matches.groups()) != 2:
        raise TargetAddressMatchingError()

    path = matches.group(2)
    if path.count('*') > 1 or ('*' in path and path[-1] != '*'):
        raise InvalidTargetAddressError(path)

    return matches.group(1), path


def process_resp_single(target_path: str, resp: Tuple[FSPResponseCode, int, bytearray], strip_name: bool = True):
    """
    Processes a single tuple with data fetched from an FSP server and writes the data to a local file.
    :param target_path: The requested path.
    :param resp: The response tuple object returned by FSPClient.get() with the data to process.
    :param strip_name: If true, the original directory is dropped and the file is saved under its name to the current
    working directory. If false, the directory structure is kept and expected to exist in the filesystem.
    :return: A return code for the script (5 when the response is Not found, 6 when the response is erroneous,
    7 when an error occurs while saving the file, 0 on success).
    """
    if resp[0] == FSPResponseCode.NOT_FOUND:
        print_err(f"The requested file '{target_path}' doesn't exist")
        return 5
    if resp[0] == FSPResponseCode.BAD_REQ or resp[0] == FSPResponseCode.ERROR:
        print_err(f"An unexpected error occurred: {resp[2].decode('utf-8')}")
        return 6
    # This ought to be safe because trying to GET a file ending with a slash would have raised an error before
    if strip_name and '/' in target_path:
        f_name = target_path[(target_path.rindex('/') + 1):]
    else:
        f_name = target_path

    try:
        with open(f_name, 'wb') as out_file:
            out_file.write(resp[2])
    except OSError as e:
        print_err(f"An unexpected error occurred while saving the file {resp[2]}: {str(e)}")
        return 7

    return 0


def process_resp_multiple(resp: Dict[str, Optional[Tuple[FSPResponseCode, int, bytearray]]]):
    """
    Processes a dictionary of tuples with data fetched from an FSP server, creates the corresponding directory structure
    in the current working directory and saves the downloaded files.
    :param resp: The response dictionary returned by FSPClient.get() with the data to process.
    :return: A return code for the script (0 if all transfers were successful, otherwise the code of the first error
    as defined in the process_resp_single() method).
    """
    ret_code = 0
    for path in sorted(resp.keys()):
        if '/' in path:
            path_without_file = '/' + path[0:path.rindex('/')]
            dir_name = path_without_file[(path_without_file.rindex('/') + 1):]
            if not os.path.exists(dir_name):
                os.mkdir(dir_name)

        ret = process_resp_single(path, resp[path], False)
        if ret != 0 and ret_code == 0:
            ret_code = ret

    return ret_code


# noinspection PyUnboundLocalVariable
def main():
    parser = argparse.ArgumentParser(description='An FSP client. Downloads a file or a directory (recursively) from '
                                                 'an FSP server. The address of the server is resolved using an NSP '
                                                 'nameserver. The saved file(s) will keep their origin name; if a '
                                                 'file already exists under the target name, it will be overwritten.',
                                     epilog='Meanings of the exit codes are described in the beginning of this script.')
    parser.add_argument('-n', nargs=1, required=True, metavar='IP:PORT',
                        help='Specifies the address and port of an NSP nameserver.')
    parser.add_argument('-f', nargs=1, required=True, metavar='SURL',
                        help='Specifies the SURL address of the required file. If the address end with an asterisk, '
                             'the whole directory is downloaded recursively. There may only be one asterisk in the '
                             'address and it must be the last character if it is present.')

    args = parser.parse_args()

    try:
        ip, _, port = args.n[0].partition(':')
        ipaddress.ip_address(ip)  # Validates the IP address
        port = int(port)
    except ValueError:
        print_err('Invalid nameserver address')
        exit(1)

    nsp = NSPClient(ip, port)

    try:
        target_name, target_path = parse_target_address(args.f[0])
    except (InvalidTargetAddressError, TargetAddressMatchingError):
        print_err('Invalid target address')
        exit(1)

    try:
        addr = nsp.resolve(target_name)
    except TimeoutError:
        print_err('Nameserver lookup timeout')
        exit(2)
    except OSError as e:
        print_err('Error while performing a nameserver query:', str(e))
        exit(2)
    except ValueError:
        print_err('Internal error occurred while reading the nameserver query response')
        exit(2)
    except InvalidNSPRequestError:
        print_err('Internal error occurred while performing a nameserver query')
        exit(2)
    except InvalidNSPResponseError as e:
        print_err('Unexpected response from the nameserver:', e)
        exit(2)

    if addr is None:
        print_err('The requested FSP server does not exist')
        exit(3)

    fsp = FSPClient(*addr, target_name)
    try:
        resp = fsp.get(target_path)
        if isinstance(resp, dict):
            ret_code = process_resp_multiple(resp)
            exit(ret_code)
        else:
            ret_code = process_resp_single(target_path, resp)
            exit(ret_code)
    except TimeoutError:
        print_err('File server response timeout')
        exit(4)
    except OSError as e:
        print_err('Error while performing a file server query:', str(e))
        exit(4)
    except ValueError as x:
        print_err('Internal error occurred while reading the file server response')
        exit(4)
    except InvalidFSPResponseError as e:
        print_err(str(e))
        exit(4)
    except UnsuccessfulFSPResponse as e:
        print_err('Cannot fetch the remote index file:', e.code.value)
        exit(4)


if __name__ == '__main__':
    main()
