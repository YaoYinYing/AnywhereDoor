from typing import List
from datastructures import ProxyConfig
from dataclasses import dataclass

# todo: add parser for config file: `.anywheredoorrc`?
predefined_proxies: List[ProxyConfig] = [
    ProxyConfig(
        "localhost",
        "10089",
        "10089",
        "oreouser",
        "ling7412",
        alternative_urls=(
           "192.168.194.143",
           "100.93.55.61",
           "z4.login1.japs.yaoyy.moe",
           "ts.login1.japs.yaoyy.moe",
           "z4.login2.japs.yaoyy.moe",
           "ts.login2.japs.yaoyy.moe",
           #"z.a100.japs.yaoyy.moe",
           #"ts.a100.japs.yaoyy.moe",
        ),
     ),
    ProxyConfig(
        "localhost",
        "10091",
        "10092",
        alternative_urls=(
            "z4.login1.japs.yaoyy.moe",
            "ts.login1.japs.yaoyy.moe",
            "z4.login2.japs.yaoyy.moe",
            "ts.login2.japs.yaoyy.moe",
            "z4.login3.japs.yaoyy.moe",
            "ts.login3.japs.yaoyy.moe",
        ),
    ),
    ProxyConfig(
        
        "192.168.194.96",
        "10089",
        "10089",
    #     alternative_urls=("ts.login1.japs.yaoyy.moe",),
    ),
]


@dataclass
class NoProxy:
    NoProxy: tuple[str] = (
        "localhost",
        "127.0.0.1",
        "10.96.0.0/12",
        "192.168.59.0/24",
        "192.168.49.0/24",
        "192.168.39.0/24",
        "192.168.67.0/24",
        "172.17.0.0/24",
        "192.168.0.0/16",
        "100.87.0.0/16",
        "192.168.75.0/24",
        "192.168.194.0/24",
        "192.168.67.2"
    )

    def __str__(self):
        return ";".join(self.NoProxy)
