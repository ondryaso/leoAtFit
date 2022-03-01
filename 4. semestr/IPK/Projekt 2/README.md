# Simple File Transfer Protokol

Autor: Ondřej Ondryáš (xondry02)\
25. dubna 2021

Tento archiv obsahuje implementaci protokolu SFTP, který je definován v RFC 913. Serverová část podporuje obsluhu více uživatelů najednou.

Klientská i serverová část byly naprogramovány v jazyce C# nad platformou .NET Core ve verzi 3.1 a byly otestovány na operačních systémech Windows 10 a Ubuntu 20.04.

## Překlad

Na 64bitovém OS Linux s nainstalovaným GNU Make je možné obě aplikace přeložit příkazem `make`. Tento příkaz vygeneruje spustitelné soubory `ipk-simpleftp-server` a `ipk-simpleftp-client`. Další možnosti překladu a spouštění poskytuje příkaz `dotnet`. Více informací najdete v dokumentaci.

## Server

Server je možné po přeložení spustit následujícím příkazem:

```
./ipk-simpleftp-server -u cesta_k_databázi_uživatelů [-i rozhraní] [-p port]
```

- Parametr `-u` je povinný a obsahuje cestu k souboru s databází uživatelů.
- Parametr `-i` je nepovinný a nastavuje název rozhraní, na jehož adrese má server naslouchat. Tento parametr je možné použít opakovaně a specifikovat tak více rozhraní. Pokud není tento parametr použit, server naslouchá na všech rozhraních.
- Parametr `-p` je nepovinný a nastavuje port, na kterém má server naslouchat. Výchozí hodnota je 115.

Příklad spuštění na Linuxu:

```
./ipk-simpleftp-server -u users -i eth0 -i eth1 -p 11500
```

Příklad spuštění pomocí *dotnet CLI* na Windows:

```
cd IpkEpsilon.Server
dotnet run -- -u ..\users.txt -i Ethernet
```

## Klient

Klienta je možné po přeložení spustit následujícím příkazem:

```
./ipk-simpleftp-client -h IP_nebo_jméno_serveru [-p port]
```

- Parametr `-h` je povinný a obsahuje IP adresu serveru, ke kterému se klient připojuje. Pokud není hodnota parametru platnou IPv4 nebo IPv6 adresou, parametr se považuje za *hostname* a je proveden pokus o přeložení na adresu pomocí DNS.
- Parametr `-p` je nepovinný a nastavuje port serveru, ke kterému se klient připojuje. Výchozí hodnota je 115.

Příklad spuštění na Linuxu:

```
./ipk-simpleftp-client -h sftp.server.org
```

Příklad spuštění pomocí *dotnet CLI* na Windows:

```
cd IpkEpsilon.Client
dotnet run -- -h 192.168.0.153 -p 11500
```

## Rozšíření oproti zadání
- Server umožňuje naslouchat na libovolném počtu rozhraní/adres.
- Klient umožňuje zadání *hostname* místo IP adresy.
- Výchozím pracovním adresářem klienta po připojení je pracovní adresář spuštěného serveru, ale server poskytuje klientům přístup k celému souborovému systému. Přijímány jsou absolutní i relativní cesty.

## Poznámky k implementaci protokolu
- Příkaz `ACCT` po úspěšně vykonaném příkazu `USER` není vyžadován, ale je přijímán a sémanticky jde o prázdnou operaci.
- Odpovědi serveru jsou kódovány pomocí UTF-8; jména souborů se speciálními (non-ASCII) znaky jsou tedy přenášena korektně (jde zejména o odpověď příkazu `LIST`). Soubor s uživatelskými jmény a hesly je však stále interpretován pouze jako ASCII!
- Přenosové režimy `BINARY` a `CONTINUOUS` jsou ekvivalentní.
- Režim uložení souboru `NEW` nepodporuje generace souborů, při jeho použití je tedy odmítnut přenos souboru, pokud cílový soubor už existuje.
- Příkaz `LIST V` vrátí nejen seznam souborů, ale i seznam podadresářů.

## Seznam souborů
- IpkEpsilon.sln
- manual.pdf
- README
- Makefile
- IpkEpsilon.Common
    - IpkEpsilon.Common.csproj
    - AsciiStreamUtils.cs
    - Base32Utils.cs
    - ConsoleUtils.cs
    - Log.cs
- IpkEpsilon.Server
    - IpkEpsilon.Server.csproj
    - Program.cs
    - Network
        - ClientHandler.cs
        - SocketServer.cs
    - Sftp
        - Abstractions
            - CommandExecutionResult.cs
            - IAccountProvider.cs
            - ISftpProvider.cs
            - ISftpProviderFactory.cs
            - NextMode.cs
            - ResponseMode.cs
        - FileAccountProvider.cs
        - SftpProvider.cs
        - SftpProviderFactory.cs
- IpkEpsilon.Client
    - IpkEpsilon.Client.csproj
    - Program.cs
    - SftpClient.cs
    - SocketClient.cs