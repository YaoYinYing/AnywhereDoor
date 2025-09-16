# AnywhereDoor
 A Linux/Python script for switching Proxies within a SSH session.

![python-version](https://img.shields.io/badge/Python-3.9_%7C_3.10_%7C_3.11_%7C_3.12-3776AB?logo=python&logoColor=yellow)
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/YaoYinYing/AnywhereDoor)

# Setup

## prerequistions
```text
pysocks
PyYAML
```
## 
1. Clone this repository
2. Run `source /repo/to/AnywhereDoor/anywhere_door.sh` and record this command in shell profile (`.bashrc`, for example)
3. Override the configuration files.
4. Call `anywhere_door` as you like!

## Configuration priorities
1. Current dir settings at `./.anywheredoorrc`, which fallbacks to:
2. User settings at `~/.anywheredoorrc`, which fallbacks to:
3. Default settings at `/repo/to/AnywhereDoor/.anywheredoorrc`

# Basic Usage
## On/Off
```bash
>>> anywhere_door
proxy selected:   Clash_Relay_0
localhost:10089/10089
```
```bash
>>> anywhere_door on
proxy selected:   Clash_Relay_0
localhost:10089/10089
```

```bash
>>> anywhere_door off
Anywhere Door is now inactive!
```

## Apply for Git
```bash
>>> anywhere_door off
Anywhere Door is now inactive!

>>> anywhere_door git
Disabling proxy for git
Done
```

```bash
>>> anywhere_door
proxy selected.
oreouser:ling7412@z4.login2.japs.yaoyy.moe:10089/10089 

>>> anywhere_door git
Enabling proxy for git
Done
```

## List all predefined proxies (raw data)
```bash
>>> anywhere_door list
Available proxies:
---------------------------------------------
1. [Clash_Relay] localhost:10089/10089 [(z1.a100.japs.yaoyy.moe, z2.a100.japs.yaoyy.moe, z4.a100.japs.yaoyy.moe, z1.login2.japs.yaoyy.moe, z2.login2.japs.yaoyy.moe, z4.login2.japs.yaoyy.moe, z1.login1.japs.yaoyy.moe, z2.login1.japs.yaoyy.moe, z4.login1.japs.yaoyy.moe)]
2. [ClashFallBack] localhost:10093/10093 [(z1.a100.japs.yaoyy.moe, z4.a100.japs.yaoyy.moe, z2.a100.japs.yaoyy.moe)]
3. [Hysterians] localhost:10091/10092 [(z1.a100.japs.yaoyy.moe, z2.a100.japs.yaoyy.moe, z4.a100.japs.yaoyy.moe, z1.login1.japs.yaoyy.moe, z2.login1.japs.yaoyy.moe, z4.login1.japs.yaoyy.moe, z1.login2.japs.yaoyy.moe, z2.login2.japs.yaoyy.moe, z4.login2.japs.yaoyy.moe)]
4. [Private] 192.168.194.96:10089/10089 [(z.yyy.mac.yaoyy.moe,)]
---------------------------------------------
```

## List all predefined proxies (usable)
```bash
>>> anywhere_door use
Testing... This could take short... Done.
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Available proxies:
---------------------------------------------------------------------------
1. [Mihomo_0] oreo:oreo@10.186.1.54:10099/10099
2. [Mihomo_1] oreo:oreo@z1.login2.japs.yaoyy.moe:10099/10099
3. [Mihomo_2] oreo:oreo@z2.login2.japs.yaoyy.moe:10099/10099
4. [Mihomo_3] oreo:oreo@z4.login2.japs.yaoyy.moe:10099/10099
5. [Singbox_0] oreo:oreo@10.186.1.54:10088/10088
6. [Singbox_1] oreo:oreo@z1.login1.japs.yaoyy.moe:10088/10088
7. [Singbox_2] oreo:oreo@z2.login1.japs.yaoyy.moe:10088/10088
8. [Singbox_3] oreo:oreo@z4.login1.japs.yaoyy.moe:10088/10088
9. [Singbox_4] oreo:oreo@z1.login2.japs.yaoyy.moe:10088/10088
10. [Singbox_5] oreo:oreo@z2.login2.japs.yaoyy.moe:10088/10088
11. [Singbox_6] oreo:oreo@z4.login2.japs.yaoyy.moe:10088/10088
12. [Singbox_7] oreo:oreo@z1.login3.japs.yaoyy.moe:10088/10088
13. [Singbox_8] oreo:oreo@z2.login3.japs.yaoyy.moe:10088/10088
14. [Singbox_9] oreo:oreo@z4.login3.japs.yaoyy.moe:10088/10088
15. [Private_0] oreo:oreo@192.168.194.98:10089/10089
16. [Private_1] oreo:oreo@100.98.177.53:10089/10089
---------------------------------------------------------------------------
Please use `anywhere_door use <index>` to pick one of them
GREEN: passed  YELLOW: in use  RED: failed
```
## Use a predefined proxy server by index
```bash
>>> anywhere_door use 1
proxy selected:   Mihomo_0
oreo:oreo@10.186.1.54:10099/10099
```

## Use a predefined proxy server by label
```bash
>>> anywhere_door use Mihomo_1
proxy selected:   Mihomo_1
oreo:oreo@z1.login2.japs.yaoyy.moe:10099/10099
```

## Show current configuration
```bash
>>> anywhere_door show
$http_proxy=socks5h://oreo:oreo@z1.login2.japs.yaoyy.moe:10099
$https_proxy=socks5h://oreo:oreo@z1.login2.japs.yaoyy.moe:10099
$all_proxy=socks5h://oreo:oreo@z1.login2.japs.yaoyy.moe:10099
```


## Proxy test
```bash
>>> anywhere_door test
Connection test to https://www.google.com: Success
Connection test to https://www.facebook.com: Success
Connection test to https://www.twitter.com: Success
Connection test to https://www.instagram.com: Success
```

## Test all proxy connections
```bash
>>> anywhere_door test all
Testing  1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
# GREEN for PASSED, RED for FAILED
```
```bash
>>> anywhere_door test full

===========================================================================
URL Testing ... ...
===========================================================================
Testing: 1: Mihomo_0
oreo:oreo@10.186.1.54:10099/10099
Connection test of https://www.google.com: Success
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
...
Testing: 15: Private_0
oreo:oreo@192.168.194.98:10089/10089
Connection test of https://www.google.com: Success
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Testing: 16: Private_1
oreo:oreo@100.98.177.53:10089/10089
Connection test of https://www.google.com: Failed
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
===========================================================================
Testing Passed: 1 3 5 7 10 15
===========================================================================
```


## Location IP check (via ipinfo.io)
```bash
>>> anywhere_door whereami
/usr/local/bin/curl
{
  "ip": "...",
  "city": "Los Angeles",
  "region": "California",
  "country": "US",
  "loc": "34.0522,-118.2437",
  "org": "....",
  "postal": "90009",
  "timezone": "America/Los_Angeles",
  "readme": "https://ipinfo.io/missingauth"

```
## Speed Test (wget)
```bash
>>> anywhere_door wget
alias wget='wget -c'
--2024-07-26 16:42:11--  https://speedtest.yaoyy-hi.workers.dev/
Resolving localhost (localhost)... 127.0.0.1
Connecting to localhost (localhost)|127.0.0.1|:10089... connected.
Proxy request sent, awaiting response... 200 OK
Length: unspecified [application/octet-stream]
Saving to: ‘/dev/null’

/dev/null                      [            <=>                        ]  34.39M  2.90MB/s
```

## Speed Test (curl)
```bash
>>> anywhere_door curl
/usr/local/bin/curl
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100 58.2M    0 58.2M    0     0  2378k      0 --:--:--  0:00:25 --:--:-- 3477k
```


## Speed Test (via speedtest commandline tool)
```bash
>>> anywhere_door bench

   Speedtest by Ookla

.....
    Download:    29.96 Mbps (data used: 40.2 MB)                                                   
                325.71 ms   (jitter: 77.17ms, low: 80.91ms, high: 550.12ms)
      Upload:    29.67 Mbps (data used: 38.3 MB)                                                   
                590.76 ms   (jitter: 86.48ms, low: 82.85ms, high: 1133.91ms)
 Packet Loss: Not available.
  Result URL: https://www.speedtest.net/result/c/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

   Speedtest by Ookla

Selecting server:
      .....
Idle Latency:    84.01 ms   (jitter: 1.32ms, low: 83.86ms, high: 88.97ms)
    Download:    30.15 Mbps (data used: 35.4 MB)                                                   
                271.96 ms   (jitter: 71.93ms, low: 81.89ms, high: 424.63ms)
      Upload:    29.92 Mbps (data used: 38.7 MB)                                                   
                565.88 ms   (jitter: 86.24ms, low: 82.68ms, high: 1148.94ms)
 Packet Loss: Not available.
  Result URL: https://www.speedtest.net/result/c/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
```

## DNS leakage test
```bash
>>> anywhere_door dns
Fetching leak ID...
Leak ID: xxxxxxx
Simulating DNS requests...
Retrieving test results...
Your IP:
111.222.333.444 [Promised Land], AS1234567 KFC Inc.
You use 1 DNS servers:
11.22.33.44 [Misfortune], AS7654321 The Anti-Fraud Center of Failure Management Company Limited
Conclusion:
DNS may be leaking.
```

## Wrap SOCKS5 to HTTP via GOST
**Require Gost installed**
```bash
# enable a door
>>> anywhere_door use 1
proxy selected:   Mihomo_0
oreo:oreo@10.186.1.54:10099/10099

# wrap to gost on default port 63322
>>> anywhere_door gost on
/usr/local/bin/gost
[1] 3132547
GOST running at 3132547

# check the new door
>>>  anywhere_door show
$http_proxy=http://127.0.0.1:63322
$https_proxy=http://127.0.0.1:63322
$all_proxy=http://127.0.0.1:63322

# or use customized port id
>>> anywhere_door gost 61234
/usr/local/bin/gost
[1] 3147411
GOST running at 3147411

# check the new door on customized port
>>> anywhere_door show
$http_proxy=http://127.0.0.1:61234
$https_proxy=http://127.0.0.1:61234
$all_proxy=http://127.0.0.1:61234

# off the gost
>>> anywhere_door gost off
/usr/local/bin/gost
Stop background GOST at 3132547
proxy selected:   Mihomo_0
oreo:oreo@10.186.1.54:10099/10099
proxy selected:   Mihomo_0
oreo:oreo@10.186.1.54:10099/10099
[1]+  Done                    gost -L=:${GOST_PORT} -F=$(anywhere_door show all | cut -b 12-) > /dev/null 2>&1

# check the original door
>>> anywhere_door show
$http_proxy=socks5h://oreo:oreo@10.186.1.54:10099
$https_proxy=socks5h://oreo:oreo@10.186.1.54:10099
$all_proxy=socks5h://oreo:oreo@10.186.1.54:10099
```

## show export commands
```bash
>>> anywhere_door export
export HTTP_PROXY=socks5h://oreo:oreo@10.186.1.54:10099
export HTTPS_PROXY=socks5h://oreo:oreo@10.186.1.54:10099
export ALL_PROXY=socks5h://oreo:oreo@10.186.1.54:10099
export NO_PROXY=localhost,127.0.0.1,192.168.0.0/16,localhost,127.0.0.1,10.96.0.0/12,192.168.59.0/24,192.168.49.0/24,192.168.39.0/24,192.168.67.0/24,172.17.0.0/24,192.168.0.0/16,100.87.0.0/16,192.168.75.0/24,192.168.194.0/24,192.168.67.2
```

## Docker daemon configs
```bash
>>> anywhere_door docker_daemon
-------------------------------------------------------------------------------
JSON
-------------------------------------------------------------------------------
{"proxies": {"default": {"httpProxy": "socks5h://oreo:oreo@10.186.1.54:10099", "httpsProxy": "socks5h://oreo:oreo@10.186.1.54:10099", "noProxy": "localhost;127.0.0.1;192.168.0.0/16;localhost;127.0.0.1;10.96.0.0/12;192.168.59.0/24;192.168.49.0/24;192.168.39.0/24;192.168.67.0/24;172.17.0.0/24;192.168.0.0/16;100.87.0.0/16;192.168.75.0/24;192.168.194.0/24;192.168.67.2"}}}

-------------------------------------------------------------------------------
SYSTEMCTL
-------------------------------------------------------------------------------
[Service]
Environment="HTTP_PROXY=socks5h://oreo:oreo@10.186.1.54:10099"
Environment="HTTPS_PROXY=socks5h://oreo:oreo@10.186.1.54:10099"
Environment="ALL_PROXY=socks5h://oreo:oreo@10.186.1.54:10099"
Environment="NO_PROXY=localhost,127.0.0.1,192.168.0.0/16,localhost,127.0.0.1,10.96.0.0/12,192.168.59.0/24,192.168.49.0/24,192.168.39.0/24,192.168.67.0/24,172.17.0.0/24,192.168.0.0/16,100.87.0.0/16,192.168.75.0/24,192.168.194.0/24,192.168.67.2"

```

## Customizing the proxy for temporary uses.
```bash
>>> anywhere_door config localhost 7890 7890
proxy selected:   Configured
localhost:7890/7890

>>> anywhere_door show
$http_proxy=http://localhost:7890
$https_proxy=http://localhost:7890
$all_proxy=socks5h://localhost:7890


>>> anywhere_door show http
$http_proxy=http://localhost:7890

>> anywhere_door show https
$https_proxy=http://localhost:7890

>>> anywhere_door show all
$all_proxy=socks5h://localhost:7890
```

## Help messages
```bash 
>>> anywhere_door ?
Anywhere Door: A quick switch for network proxies in the current session.
Usage: anywhere_door [command]
Commands:
   on        : Activate Anywhere Door
   off       : Deactivate Anywhere Door
   config    : Configure custom IP and port
   show      : Show the current proxy configurations
   export    : Show the quick export lines for shell
   list/ls   : Show all predefined proxies
   test      : Perform a test connection to check proxy accessibility
   bench     : Perform a speed test connection to check network bandwidth
   wget      : Perform a speed test connection via wget to check network bandwidth
   curl      : Perform a speed test connection via curl to check network bandwidth
   whereami  : Perform a location check via curl
   use       : Use a specific proxy from the configured list
   upgrade   : Upgrade to the latest code
   dns/leak  : DNS Leak Test
   docker_daemon: Generate HTTP proxy setup for docker daemon
   gost      : Wrap Socks to HTTP by GOST
   help/?    : Show this help message and exit
```

```bash 
>>> anywhere_door ?  use
Call a predefined proxy.
Usage: anywhere_door use [opt]
   <empty>   : Show all predefined proxies, colored with url test results.
   [index]   : Set indexed proxy.
   [label]   : Set label proxy.
```


```bash 
>>> anywhere_door ? test
Testing proxies.
Usage: anywhere_door test [opt]
   <empty>   : Test current proxy.
   all       : Test all predefined proxies, simplified results.
   full      : Test all predefined proxies, detailed results.
```

```bash
>>> anywhere_door ? config
Configure a new proxy.
Usage: anywhere_door config server_url http_port socks_port [username] [password]
```

```bash
>>> anywhere_door ? show
Show current proxy.
Usage: anywhere_door show [proxy_type: 'http', 'https', 'all']
```

# Advanced
**AnywhereDoor + SSH Remote Forwarding**

## SSH Remote Forwarding

1. Add HTTP/Socks proxy port on local machine.
2. Connect server on SSH with remote port forwarding:
   ```shell
   # forward proxy of localhost:7890 to remote-server:37890
   ssh -R 37890:localhost:7890 remote-server
   ```
3. Use forwarded port as proxy config.
   ```shell
   # On remote server, use forwarded proxy port 37890
   anywhere_door config localhost 37890 37890
   ```
