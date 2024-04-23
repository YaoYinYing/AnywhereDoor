# AnywhereDoor
 A Linux/Python script for switching Proxies within a SSH session.

# Setup
1. Clone this repository
2. Run `source /repo/to/AnywhereDoor/anywhere_door.sh` and record this command in shell profile (`.bashrc`, for example)
3. Modify the configuration in `AnywhereDoor/predefined_proxies.py`
4. Call `anywhere_door` as you like!

# Basic Usage
## On/Off
```bash
>>> anywhere_door
proxy selected.
oreouser:ling7412@z4.login2.japs.yaoyy.moe:10089/10089 
```
```bash
>>> anywhere_door on
proxy selected.
oreouser:ling7412@z4.login2.japs.yaoyy.moe:10089/10089 
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

## List all predefined proxies
```bash
>>> anywhere_door use
Available proxies:
---------------------------------------------
1. oreouser:ling7412@z4.login2.japs.yaoyy.moe:10089/10089 
2. oreouser:ling7412@ts.login2.japs.yaoyy.moe:10089/10089 
3. oreouser:ling7412@z4.login3.japs.yaoyy.moe:10089/10089 
4. oreouser:ling7412@ts.login3.japs.yaoyy.moe:10089/10089 
5. z4.login2.japs.yaoyy.moe:10091/10092 
6. ts.login2.japs.yaoyy.moe:10091/10092 
7. z4.login1.japs.yaoyy.moe:10091/10092 
8. ts.login1.japs.yaoyy.moe:10091/10092 
9. z4.login1.japs.yaoyy.moe:10089/10089 
10. ts.login1.japs.yaoyy.moe:10089/10089 
---------------------------------------------
Please use `anywhere_door use <index>` to pick one of them.
```
## Use a predefined proxy server
```bash
>>> anywhere_door use 2
proxy selected.
oreouser:ling7412@ts.login2.japs.yaoyy.moe:10089/10089 
```

## Show current configuration
```bash
>>> anywhere_door show
$http_proxy=http://ts.login1.japs.yaoyy.moe:10089
$https_proxy=http://ts.login1.japs.yaoyy.moe:10089
$all_proxy=socks5://ts.login1.japs.yaoyy.moe:10089
```


## Proxy test
```bash
>>> anywhere_door test
Connection test to https://www.facebook.com: Success  Time taken: 7.13 seconds
Connection test to https://www.google.com: Success  Time taken: 1.86 seconds
Connection test to https://www.twitter.com: Success  Time taken: 1.67 seconds
Connection test to https://www.instagram.com: Success  Time taken: 2.99 seconds

All test connections passed successfully. The tested websites are accessible.
```

## Test all proxy connections
```bash
>>> nywhere_door test all
Testing 1 ...
proxy selected.
oreouser:ling7412@z4.login2.japs.yaoyy.moe:10089/10089 
Connection test to https://www.google.com: Success  Time taken: 6.71 seconds
.......
Testing 10 ...
proxy selected.
ts.login1.japs.yaoyy.moe:10089/10089 
Connection test to https://www.google.com: Success  Time taken:  seconds
Testing Passed: 1 2 3 4 5 6 7 8 9 10
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
  Result URL: https://www.speedtest.net/result/c/900e058a-8197-442f-91ae-5ff3bc03bc88

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

## Customizing the proxy for temporary uses.
```bash
>>> anywhere_door config localhost 7890 7890
proxy selected.
localhost:7890/7890 

>>> anywhere_door show
$http_proxy=http://localhost:7890
$https_proxy=http://localhost:7890
$all_proxy=socks5://localhost:7890
```

## Help messages
```bash 
>>> anywhere_door ?
Anywhere Door: A quick switch for network proxies in the current session.
Usage: anywhere_door [command]
Commands:
 on : Activate Anywhere Door
 off : Deactivate Anywhere Door
 config : Configure custom IP and port
 show : Show the current proxy configurations
 test : Perform a test connection to check proxy accessibility
 bench : Perform a speed test connection to check network bandwith
 use : Use a specific proxy from the configured list
 help/? : Show this help message and exit
```

```bash 
>>> anywhere_door ? use
Call a predefined proxy. 
Usage: anywhere_door use [opt]
 <empty> : Show all predefined proxies.
 [index] : Set indexed proxy.
```


```bash 
>>> anywhere_door ? test
Testing proxies. 
Usage: anywhere_door test [opt]
 <empty> : Test current proxy.
 all : Test all predefined proxies.
```

```bash
>>> anywhere_door help config
Config a new proxy. 
Usage: anywhere_door config server_url http_port socks_port [username] [password]
```



