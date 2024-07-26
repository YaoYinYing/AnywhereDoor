from dataclasses import dataclass
from typing import List, Literal


ProxyType=Literal["http", "https", "all"]

@dataclass
class ProxyConfig:
    url: str
    http_port: str
    socks_port: str
    authentication_user: str = None
    password: str = None
    alternative_urls: tuple[str] = tuple()

    @property
    def http_proxy(self) -> str:
        if self.authentication_user and self.password:
            return f"http://{self.authentication_user}:{self.password}@{self.url}:{self.http_port}"
        else:
            return f"http://{self.url}:{self.http_port}"

    @property
    def socks_proxy(self) -> str:
        if self.authentication_user and self.password:
            return f"socks5h://{self.authentication_user}:{self.password}@{self.url}:{self.socks_port}"
        else:
            return f"socks5h://{self.url}:{self.socks_port}"

    def __str__(self) -> str:
        return f"{f'{self.authentication_user}:{self.password}@' if self.authentication_user else ''}{self.url}:{self.http_port}/{self.socks_port} {f'[{self.alternative_urls }]' if self.alternative_urls else ''}"

    @property
    def expanded(self) -> List["ProxyConfig"]:
        _ = []
        for url in [self.url] + list(self.alternative_urls):
            __ = ProxyConfig(
                url=url,
                http_port=self.http_port,
                socks_port=self.socks_port,
                authentication_user=self.authentication_user,
                password=self.password,
                alternative_urls=tuple(),
            )
            _.append(__)
        return _


@dataclass
class ProxyTable:
    proxies: List[ProxyConfig]

    @property
    def expanded(self) -> List[ProxyConfig]:
        proxies: List[ProxyConfig] = []
        for p in self.proxies:
            proxies.extend(p.expanded)
        return proxies
