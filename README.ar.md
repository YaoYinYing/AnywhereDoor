# على أي حال

برنامج نصي Linux/Python لتبديل الوكلاء داخل جلسة SSH.

![python-version](https://img.shields.io/badge/Python-3.9_%7C_3.10_%7C_3.11_%7C_3.12-3776AB?logo=python&logoColor=yellow)

# يثبت

1.  استنساخ هذا المستودع
2.  يجري`source /repo/to/AnywhereDoor/anywhere_door.sh`وتسجيل هذا الأمر في ملف shell (`.bashrc`، على سبيل المثال)
3.  تعديل التكوين في`AnywhereDoor/predefined_proxies.py`
4.  يتصل`anywhere_door`كما تحب!

# الاستخدام الأساسي

## ON/OFF

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

## تقدم بطلب للحصول على git

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

## سرد جميع الوكلاء المحدد مسبقًا (البيانات الأولية)

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

## قائمة جميع الوكلاء المحدد مسبقا (قابلة للاستخدام)

```bash
>>> anywhere_door use
Testing... This could take short... Done.
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Available proxies:
---------------------------------------------------------------------------
1. [Clash_0] localhost:10089/10089
2. [Clash_1] z1.a100.japs.yaoyy.moe:10089/10089
3. [Clash_2] z2.a100.japs.yaoyy.moe:10089/10089
4. [Clash_3] z4.a100.japs.yaoyy.moe:10089/10089
5. [Clash_4] z1.login2.japs.yaoyy.moe:10089/10089
6. [Clash_5] z2.login2.japs.yaoyy.moe:10089/10089
7. [Clash_6] z4.login2.japs.yaoyy.moe:10089/10089
8. [Clash_7] z1.login1.japs.yaoyy.moe:10089/10089
9. [Clash_8] z2.login1.japs.yaoyy.moe:10089/10089
10. [Clash_9] z4.login1.japs.yaoyy.moe:10089/10089
11. [ClashFallBack_0] localhost:10093/10093
12. [ClashFallBack_1] z1.a100.japs.yaoyy.moe:10093/10093
13. [ClashFallBack_2] z2.a100.japs.yaoyy.moe:10093/10093
14. [ClashFallBack_3] z4.a100.japs.yaoyy.moe:10093/10093
15. [ClashFallBack_4] z1.login2.japs.yaoyy.moe:10093/10093
16. [ClashFallBack_5] z2.login2.japs.yaoyy.moe:10093/10093
17. [ClashFallBack_6] z4.login2.japs.yaoyy.moe:10093/10093
18. [ClashFallBack_7] z1.login1.japs.yaoyy.moe:10093/10093
19. [ClashFallBack_8] z2.login1.japs.yaoyy.moe:10093/10093
20. [ClashFallBack_9] z4.login1.japs.yaoyy.moe:10093/10093
21. [Private_0] 192.168.194.96:10089/10089
22. [Private_1] z.yyy.mac.yaoyy.moe:10089/10089
---------------------------------------------------------------------------
Please use `anywhere_door use <index>` to pick one of them
GREEN: passed  YELLOW: in use  RED: failed
```

## استخدم خادم وكيل محدد مسبقًا عن طريق الفهرس

```bash
>>> anywhere_door use 2
proxy selected:   Clash_Relay_1
z1.a100.japs.yaoyy.moe:10089/10089
```

## استخدم خادم وكيل محدد مسبقًا عن طريق التسمية

```bash
>>> anywhere_door use Hysterians_5
proxy selected:   Hysterians_5
z2.login1.japs.yaoyy.moe:10091/10092
```

## إظهار التكوين الحالي

```bash
>>> anywhere_door show
$http_proxy=http://z2.login1.japs.yaoyy.moe:10091
$https_proxy=http://z2.login1.japs.yaoyy.moe:10091
$all_proxy=socks5h://z2.login1.japs.yaoyy.moe:10092
```

## اختبار الوكيل

```bash
>>> anywhere_door test
Connection test to https://www.facebook.com: Success
Connection test to https://www.google.com: Success
Connection test to https://www.twitter.com: Success
Connection test to https://www.instagram.com: Success
```

## اختبار جميع اتصالات الوكيل

```bash
>>> anywhere_door test all
Testing  1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22
# GREEN for PASSED, RED for FAILED
```

```bash
>>> anywhere_door test full

===========================================================================
URL Testing ... ...
===========================================================================
Testing: 1: Clash_0
localhost:10089/10089
Connection test of https://www.facebook.com: Success
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Testing: 2: Clash_1
z1.a100.japs.yaoyy.moe:10089/10089
Connection test of https://www.facebook.com: Success
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Testing: 3: Clash_2
z2.a100.japs.yaoyy.moe:10089/10089
Connection test of https://www.facebook.com: Success
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
...
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Testing: 21: Private_0
192.168.194.96:10089/10089
Connection test of https://www.facebook.com: Success
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Testing: 22: Private_1
z.yyy.mac.yaoyy.moe:10089/10089
Connection test of https://www.facebook.com: Failed
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
===========================================================================
Testing Passed: 1 2 3 4 11 12 13 14 21
===========================================================================
```

## فحص IP للموقع (عبر ipinfo.io)

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

## اختبار السرعة (wget)

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

## اختبار السرعة (حليقة)

```bash
>>> anywhere_door curl
/usr/local/bin/curl
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100 58.2M    0 58.2M    0     0  2378k      0 --:--:--  0:00:25 --:--:-- 3477k
```

## اختبار السرعة (عبر أداة سطر الأوامر Speedtest)

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

## اختبار تسرب DNS

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

## تخصيص الوكيل للاستخدامات المؤقتة.

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

## مساعدة الرسائل

```bash
>>>  anywhere_door ?
Anywhere Door: A quick switch for network proxies in the current session.
Usage: anywhere_door [command]
Commands:
   on        : Activate Anywhere Door
   off       : Deactivate Anywhere Door
   config    : Configure custom IP and port
   show      : Show the current proxy configurations
   list/ls   : Show all predefined proxies
   test      : Perform a test connection to check proxy accessibility
   bench     : Perform a speed test connection to check network bandwidth
   wget      : Perform a speed test connection via wget to check network bandwidth
   curl      : Perform a speed test connection via curl to check network bandwidth
   whereami  : Perform a location check via curl
   use       : Use a specific proxy from the configured list
   upgrade   : Upgrade to the latest code
   dns/leak  : DNS Leak Test
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

# متقدم

**anyheredoor + ssh إعادة التوجيه عن بعد**

## SSH إعادة توجيه عن بعد

1.  إضافة منفذ الوكيل HTTP/Socks على الجهاز المحلي.
2.  قم بتوصيل الخادم على SSH مع إعادة توجيه المنفذ عن بعد:
    ```shell
    # forward proxy of localhost:7890 to remote-server:37890
    ssh -R 37890:localhost:7890 remote-server
    ```
3.  استخدام المنفذ المعاد توجيهه كتكوين وكيل.
    ```shell
    # On remote server, use forwarded proxy port 37890
    anywhere_door config localhost 37890 37890
    ```
