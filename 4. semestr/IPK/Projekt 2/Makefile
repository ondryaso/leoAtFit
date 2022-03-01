.PHONY: both server client

both: server client

server: IpkEpsilon.Server/IpkEpsilon.Server.csproj
	dotnet publish IpkEpsilon.Server/IpkEpsilon.Server.csproj -c Release -o build --self-contained false -p:PublishSingleFile=true -r linux-x64
	cp -f build/IpkEpsilon.Server ./ipk-simpleftp-server

client: IpkEpsilon.Client/IpkEpsilon.Client.csproj
	dotnet publish IpkEpsilon.Client/IpkEpsilon.Client.csproj -c Release -o build --self-contained false -p:PublishSingleFile=true -r linux-x64
	cp -f build/IpkEpsilon.Client ./ipk-simpleftp-client


