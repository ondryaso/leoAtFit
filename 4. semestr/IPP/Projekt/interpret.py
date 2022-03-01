#!/usr/bin/python

# IPP project, task 2, part 1: interpret.py
# File:   interpret.py
# Author: Ondřej Ondryáš (xondry02@stud.fit.vubr.cz)
# Date:   2021-04-11

import argparse
import re
import sys
import traceback
from typing import Optional, List

from exceptions import XmlMalformedError, XmlProgramFormatError, InvalidOpcodeError, LexicalError, InterpretError, \
    InterpreterContext, SourceNotLoadedError, UndefinedLabelError, UndefinedFrameError, RedefinedVariableError, \
    UndefinedVariableError, InvalidTypeError
from data_structures import Variable, InterpreterContext
from instruction_definitions import instruction_defs, Instruction
from interpret_utils import eprint

# noinspection PyPep8Naming
import xml.etree.ElementTree as ET


def main():
    """Interpreter entry point"""

    parser = argparse.ArgumentParser(
        description='Loads an XML representation of an IPPcode21 program and interprets it.',
        epilog='At least one of --source and --input arguments must be used. If one of them is missing, '
               'the corresponding input will be read from stdin.')
    parser.add_argument('-s', '--source', nargs=1, help='path to the input program file', action='append')
    parser.add_argument('-i', '--input', nargs=1, help='path to a file with input for the interpreted program',
                        action='append')

    args = None
    try:
        args = parser.parse_args()
    except SystemExit:
        if '--help' in sys.argv:
            exit(0)
        else:
            exit(10)

    if args is None or (args.source is None and args.input is None):
        eprint('Source XML or input file must be supplied.')
        exit(10)

    if (args.source is not None and len(args.source) > 1) or (args.input is not None and len(args.input) > 1):
        eprint('Only one occurrence of --source or --input is allowed.')
        exit(10)

    try:
        interpret = Interpreter(None if args.input is None else args.input[0][0])
        interpret.load_source(None if args.source is None else args.source[0][0])
        exit_code = interpret.process()
    except OSError as e:
        eprint('Error opening one of the input files:', e.strerror, f"({e.filename})")
        exit(11)  # This script only opens input files
    except (XmlMalformedError, XmlProgramFormatError, InvalidOpcodeError, LexicalError) as e:
        eprint(e)
        exit(e.exit_code)
    except InterpretError as e:
        eprint(e)
        e.context.dump()
        exit(e.exit_code)
    except:  # A catch-all clause is used to change the exit code to 99
        traceback.print_exc()
        exit(99)

    exit(exit_code)


class Interpreter:
    """This class provides interface to the interpreter as a whole. An instance of this class encapsulates
    a single program run: it loads a source file and processes the instructions."""

    var_name_regex = re.compile(r'^(GF|TF|LF)@([a-zA-Z_$&%*!?-][a-zA-Z0-9_$&%*!?-]*)[ \t]*$')

    def __init__(self, input_file: Optional[str]):
        """
        Interpret constructor. A path to a file with data to use as the interpreted program's input may be supplied.
        If it is not provided, stdin is used.

        :param input_file: Path to an input data file, or None to use stdin.
        """
        self.xml: Optional[ET.ElementTree] = None
        self.loaded = False

        if input_file is None:
            self.in_f = sys.stdin
        else:
            self.in_f = open(input_file, 'r', encoding='utf-8')

    def load_source(self, source_file: Optional[str]):
        """Loads a program source file in the XML format. If a None is provided, the input XML is read from stdin.
        In that case, the program input data file must have been specified in constructor.

        :param source_file: Path to a program source in the XML format, or None to use stdin.
        :raises ValueError: Raised when both program input data and program source are set to be loaded from stdin.
        :raises XmlMalformedError: Raised when parsing the program source XML fails."""
        if self.in_f is sys.stdin and source_file is None:
            # Shouldn't happen
            raise ValueError('Cannot use stdin as input for both the program and the program input.')

        if source_file is None:
            self.xml = ET.parse(sys.stdin)
        else:
            src = open(source_file, 'r', encoding='utf-8')
            try:
                self.xml = ET.parse(src)
            except ET.ParseError as e:
                src.close()
                raise XmlMalformedError(e.position) from e

            src.close()

        self.loaded = True

    def process(self) -> int:
        """Performs the program interpretation. Sorts the instructions, prepares an InterpreterContext and processes
        the instructions. Program source must have been loaded using load_source().

        :return: Exit code of the interpreted program.
        :raises SourceNotLoadedError: Raised when a source file isn't loaded.
        :raises XmlProgramFormatError: Raised when the structure of the source XML is invalid, e.g. when there's
        an unexpected element or attribute.

        :raises InvalidOpcodeError: Raised when there's an instruction with unknown opcode.
        :raises LexicalError: Raised when there's a lexically invalid token, e.g. an invalid variable name.
        :raises InterpretError: InterpretError and its derivatives signalise different invalid operations
        in the program execution. See the 'instruction_definitions' module for more information.
        """
        if not self.loaded:
            raise SourceNotLoadedError()

        root = self.xml.getroot()
        if root.get('language') != 'IPPcode21':
            raise XmlProgramFormatError('Missing language specification')
        if len(root.attrib.keys() - ['language', 'name', 'description']) > 0:
            raise XmlProgramFormatError('Invalid attributes found in the root element')

        all_instructions = list(root)
        self.__check_input_instructions(all_instructions)

        instr_sorted = sorted(all_instructions, key=lambda element: int(element.get('order')))
        context = InterpreterContext(self.in_f)
        instr_count = len(instr_sorted)

        while context.exit_code == -1:
            if context.program_counter >= instr_count:
                if context.current_jump_target is None:
                    # Reached end of program successfully
                    return 0
                else:
                    # Reaching the end when current_jump_target is not None means that
                    # we were looking for a label and it failed. Find the corresponding JUMP-like instruction
                    # and raise an error.
                    start = instr_sorted[context.current_jump_start_pc]
                    opcode = start.get('opcode').upper().strip()
                    i = instruction_defs[opcode]
                    raise UndefinedLabelError(context, i, context.current_jump_target)

            self.__process_instruction(context, instr_sorted[context.program_counter])

        return context.exit_code

    @classmethod
    def __check_input_instructions(cls, input_instructions: List[ET.Element]):
        """Iterates through the instruction elements of the program source XML and checks its structure.
        Raises an error to signalise a problem with it.

        :param input_instructions: List of <instruction> elements.
        :raises XmlProgramFormatError: Raised when there's a problem with the XML structure.
        :raises InvalidOpcodeError: Raised when there's an instruction with unknown opcode.
        """
        allowed_instruction_attributes = ['order', 'opcode']
        op_numbers = []

        for i in input_instructions:
            # This list is redefined in each cycle because we're removing its values to check for
            # multiple occurrences of an argument and for occurrences of arguments defined without having
            # their predecessors also defined.
            allowed_arg_elements = ['arg1', 'arg2', 'arg3']

            if i.tag != 'instruction':
                raise XmlProgramFormatError('Unexpected element', i.tag)

            order = i.get('order')
            if order is None:
                raise XmlProgramFormatError("Found an 'instruction' element with no 'order' attribute")
            try:
                order = int(order)
                if order <= 0:
                    raise XmlProgramFormatError("Invalid value of the 'order' attribute (expected 1 or more)", order)
                if order in op_numbers:
                    raise XmlProgramFormatError('Duplicate instruction order', order)

                op_numbers.append(order)
            except ValueError as e:
                # This _could_ be a LexicalError
                raise XmlProgramFormatError("Invalid value of the 'order' attribute", order) from e

            opcode = i.get('opcode')
            if opcode is None:
                raise XmlProgramFormatError("Found an 'instruction' element with no 'opcode' attribute")
            opcode = opcode.upper().strip()
            if opcode not in instruction_defs:
                raise InvalidOpcodeError(opcode, order)

            if len(i.attrib.keys() - allowed_instruction_attributes) > 0:
                raise XmlProgramFormatError('Invalid attributes found in an instruction element', order, opcode)

            instruction_def = instruction_defs[opcode]

            for arg in i:
                if arg.tag not in allowed_arg_elements:
                    raise XmlProgramFormatError(f"Unexpected child '{arg.tag}' of an instruction element", order,
                                                opcode)
                allowed_arg_elements.remove(arg.tag)
                arg_type = arg.get('type')
                if arg_type is None:
                    raise XmlProgramFormatError(f"Found an '{arg.tag}' element with no 'type' attribute")

                if arg_type == "var":
                    if cls.var_name_regex.fullmatch(arg.text) is None:
                        raise LexicalError(arg.text, 'variable identifier', instruction_def.name, order)
                else:  # If this is a constant, try to parse its value to catch possible lexical errors
                    temp_var = Variable()
                    try:
                        temp_var.set_value_parse(arg_type, arg.text)
                    except LexicalError as le:
                        raise LexicalError(le.invalid_literal, le.expected_type, instruction_def.name,
                                           order) from le
                    except InvalidTypeError:
                        raise LexicalError(arg_type, 'variable type', instruction_def.name, order)

                if len(arg) != 0:
                    raise XmlProgramFormatError(f"Found an '{arg.tag}' element with children elements", order, opcode)

            if 'arg2' not in allowed_arg_elements and 'arg1' in allowed_arg_elements:
                raise XmlProgramFormatError(f"Found an arg2 element when there is no arg1 element")

            if 'arg3' not in allowed_arg_elements and 'arg2' in allowed_arg_elements:
                raise XmlProgramFormatError(f"Found an arg3 element when there is no arg2 element")

            exp_arg_count = instruction_def.argument_count()
            actual_arg_count = 3 - len(allowed_arg_elements)
            if actual_arg_count != exp_arg_count:
                raise XmlProgramFormatError(f"Invalid number of arguments for instruction '{opcode}' "
                                            + f"(expected {exp_arg_count}, found {actual_arg_count}).")

    @staticmethod
    def __process_instruction(context: InterpreterContext, input_instruction: ET.Element):
        """Processes a single instruction element. Parses its arguments, resolves variables and labels and
        calls the corresponding instruction process method.

        :param context: InterpreterContext object encapsulating the current interpreter state.
        :param input_instruction: Instruction element.
        :raises LexicalError: Raised when there's a lexically invalid token, e.g. an invalid variable name.
        :raises UndefinedVariableError
        :raises RedefinedVariableError
        :raises InterpretError
        """
        opcode = input_instruction.get('opcode').upper().strip()
        i = instruction_defs[opcode]

        # DEFVARs are special. For other instructions, the interpreter first resolves arguments into
        # Variable objects and then passes them to the instruction's process method. However, this would obviously
        # fail for DEFVARs, because the variables haven't been defined yet.
        if opcode == 'DEFVAR':
            # Don't do any of that if we're only passing through the code to find a label
            if context.current_jump_target is None:
                arg = input_instruction.find('arg1')  # Existence of arg1 has been checked in __check_input_instructions
                Interpreter.__process_argument(context, i, arg, True)
            context.program_counter += 1
            return

        # When opcode is LABEL, the instruction will be processed normally.
        # If the newly added label is the current jump target, the current_jump_target variable will be
        # set to None by the add_label() method in the context, PC will be incremented and the program will go on.
        if context.current_jump_target is not None and i != instruction_defs['LABEL']:
            context.program_counter += 1
            return

        args = Interpreter.__process_arguments(context, input_instruction, i)

        # TODO: SUGGESTION: Enhance exceptions with current context if it's None
        i.process(context, args)

        if context.performing_lookup_jump and context.current_jump_target is None:
            context.program_counter = context.current_jump_start_pc
            context.performing_lookup_jump = False
        else:
            context.program_counter += 1

    @staticmethod
    def __process_arguments(context: InterpreterContext, input_instruction: ET.Element,
                            instruction_def: Instruction):
        """Extracts <argX> children from an <instruction> element from the input XML, processes them
        and returns a list of Variable objects."""

        args = [input_instruction.find('arg1'), input_instruction.find('arg2'), input_instruction.find('arg3')]
        args = [Interpreter.__process_argument(context, instruction_def, a) for a in args if a is not None]
        return args

    @classmethod
    def __process_argument(cls, context: InterpreterContext, instruction_def: Instruction,
                           input_argument: ET.Element, def_var: bool = False):
        """Processes an <argX> element from the input XML. This method creates or resolves a Variable instance
        that represents the argument and is passed to the instruction processing method.

        :param context: The current execution context.
        :param instruction_def: The target Instruction object (used in exceptions).
        :param input_argument: The input argument element to process.
        :param def_var: If true, always attempts to define a new variable in the specified frame. If false,
        always attempts to resolve an existing variable from the current frame.

        :return: A resolved Variable (either a program variable or a constant).
        """
        input_type = input_argument.get('type')

        if input_type == 'var':
            literal = input_argument.text
            reg_match = cls.var_name_regex.fullmatch(literal)

            if reg_match is None:
                raise LexicalError(input_argument.text, 'variable identifier', instruction_def.name,
                                   context.program_counter)

            fr_def = reg_match.group(1)
            var_name = reg_match.group(2)

            if fr_def == 'GF':
                frame = context.global_frame
            elif fr_def == 'LF':
                frame = context.local_frame
            elif fr_def == 'TF':
                frame = context.temp_frame
            else:
                raise LexicalError(fr_def, "frame specification", instruction_def.name,
                                   context.program_counter)

            if frame is None:
                raise UndefinedFrameError(context, instruction_def, fr_def)

            if def_var and frame.has_var(var_name):
                raise RedefinedVariableError(context, instruction_def, literal)
            elif def_var:
                return frame.def_var(var_name)
            elif frame.has_var(var_name):
                return frame.get_var(var_name)
            else:
                raise UndefinedVariableError(context, instruction_def, literal)
        else:
            const_var = Variable()
            try:
                const_var.set_value_parse(input_type, input_argument.text)
            except LexicalError as le:
                raise LexicalError(le.invalid_literal, le.expected_type, instruction_def.name,
                                   context.program_counter) from le
            except InvalidTypeError:
                raise LexicalError(input_type, 'variable type', instruction_def.name, context.program_counter)

            return const_var

    def __del__(self):
        """Interpreter destructor. If this instance reads program input from a file, the file handle is closed."""

        if hasattr(self, 'in_f') and self.in_f is not sys.stdin:
            self.in_f.close()


if __name__ == '__main__':
    main()
