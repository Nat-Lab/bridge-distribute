bridge-distribute
---

`bridge-distribute` is distribution server for `ns3-distributed-bridge`. 

### Install

Simply Colne and run `make` to build the project.

```
$ git clone https://github.com/Nat-Lab/bridge-distribute
$ make
```

### Usage

Use `./dist-server_tcp` to start the server. Configurable options are listed below:

```
usage: ./dist-server_tcp -h
usage: ./dist-server_tcp [-b bind] [-p port] [-m mode]
where: mode := { switch | hub | stream }
```

- `-b`: Address to listen on. (Default: 0.0.0.0)
- `-p`: Port to listen on. (Default: 2672)
- `-m`: Mode to use, Available modes are listed below. (Default: Switch)

Modes:

- __Switch__: Act like a switch. MAC address learning will be enabled.
- __Hub__: Act like a hub. No MAC address learning. Every member of a network will receive traffic send by others in the same network.
- __Stream__: In hub and switch mode, the distribution server waits until it gets the full ethernet frame before forwarding the traffic to member ports. Stream mode disables this and traffic received will be imminently forward to other members in the same network. No MAC address learning for this mode for obvious reason.

### License

UNLICENSE