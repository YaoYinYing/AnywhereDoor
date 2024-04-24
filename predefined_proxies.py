from typing import List
from datastructures import ProxyConfig

# todo: add parser for config file: `.anywheredoorrc`?
predefined_proxies: List[ProxyConfig] = [
    ProxyConfig(
        "z4.login2.japs.yaoyy.moe",
        "10089",
        "10089",
        "oreouser",
        "ling7412",
        alternative_urls=(
            "ts.login2.japs.yaoyy.moe",
            "z4.login3.japs.yaoyy.moe",
            "ts.login3.japs.yaoyy.moe",
        ),
    ),
    ProxyConfig(
        "z4.login2.japs.yaoyy.moe",
        "10091",
        "10092",
        alternative_urls=(
            "ts.login2.japs.yaoyy.moe",
            "z4.login1.japs.yaoyy.moe",
            "ts.login1.japs.yaoyy.moe",
        ),
    ),
    ProxyConfig(
        "z4.login1.japs.yaoyy.moe",
        "10089",
        "10089",
        alternative_urls=("ts.login1.japs.yaoyy.moe",),
    ),
]
