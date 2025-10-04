from dataclasses import dataclass
from typing import List, Literal, Mapping
import concurrent.futures

from urllib import request, error


ProxyType = Literal["http", "https", "all"]

def url_tests(url: str, proxies: Mapping, timeout: int = 500) -> bool:
    # Prepare the proxy handler
    proxy_handler = request.ProxyHandler(proxies)
    opener = request.build_opener(proxy_handler)
    request.install_opener(opener)

    flag:bool=False
    try:
        # Open the URL with the specified timeout
        with request.urlopen(url, timeout=timeout/1000) as response:
            # Check if the request was successful (HTTP status code 200)
            flag=response.status == 200
            #print(f'Flag: {flag}, Response: {response.status}: {response.text}')
    except (error.URLError, error.HTTPError) as e:
        # Catch URL and HTTP errors
        flag = False
        #print(f'Flag: {flag}, Error: {e}')

    finally:
        opener.close()
        return flag


def test_urls_concurrently(urls: List[str], proxies: Mapping, timeout: int) -> Mapping[str,bool]:
    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = {
            executor.submit(url_tests, url, proxies, timeout): url for url in urls
        }
        results = {}
        for future in concurrent.futures.as_completed(futures):
            try:
                url=futures[future]
                success = future.result()
                results.update({url: success})
            except Exception as e:
                results.update({url: False})
        return {k:results.get(k) for k in urls}

def test_proxies_concurrently(proxies: tuple["ProxyConfig"]) -> Mapping["ProxyConfig", bool]:
    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = {
            executor.submit(url_tests, proxy.test_urls[0], {'http': proxy.http_proxy, 'https': proxy.socks_proxy}, proxy.test_timeout): proxy for proxy in proxies
        }
        results = {}
        for future in concurrent.futures.as_completed(futures):
            try:
                proxy=futures[future]
                success = future.result()
                results.update({proxy: success})

            except Exception as e:
                results.update({proxy: False})

        return {k:results.get(k) for k in proxies}
    
@dataclass(frozen=True)
class ProxyConfig:
    url: str
    http_port: str
    socks_port: str
    authentication_user: str = None
    password: str = None
    alternative_urls: tuple[str] = tuple()
    label: str = "Default"

    test_urls: tuple[str] = (
        #"https://speedtest.yaoyy-hi.workers.dev/10K",
        "https://www.google.com",
        "https://www.facebook.com",
        "https://www.twitter.com",
        "https://www.instagram.com",
    )
    test_timeout: int = 10_000 # Microseconds

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
        if not (
            isinstance(self.alternative_urls, (list, tuple))
            and len(self.alternative_urls) > 0
        ):
            return [self]

        _ = []
        all_urls = [self.url] + list(self.alternative_urls)
        for i, url in enumerate(all_urls):
            __ = ProxyConfig(
                url=url,
                http_port=self.http_port,
                socks_port=self.socks_port,
                authentication_user=self.authentication_user,
                password=self.password,
                label=f"{self.label}_{i}",
            )
            _.append(__)
        return _

    def is_available(self, url):
        proxies = {
            "http": self.http_proxy,
            "https": self.socks_proxy,
        }
        return url_tests(url=url, proxies=proxies, timeout=self.test_timeout)

    # def __bool__(self):
    #     return self.is_available(url=self.test_urls[0])

    def __eq__(self, other: "ProxyConfig"):
        return (
            self.url == other.url
            and self.http_port == other.http_port
            and self.socks_port == other.socks_port
            and self.authentication_user == other.authentication_user
            and self.password == other.password
        )


@dataclass
class ProxyTable:
    proxies: List[ProxyConfig]

    @property
    def expanded(self) -> List[ProxyConfig]:
        proxies: List[ProxyConfig] = []
        for p in self.proxies:
            proxies.extend(p.expanded)
        return proxies


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
        "192.168.67.2",
    )

    def __str__(self):
        return ";".join(self.NoProxy)
