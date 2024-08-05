from dataclasses import dataclass
from typing import List, Literal, Mapping
import concurrent.futures


import urllib3


ProxyType = Literal["http", "https", "all"]


def url_tests(url: str, proxies: Mapping = None, timeout: int = 500) -> bool:
    # Create a PoolManager instance
    http = urllib3.PoolManager()

    # Configure the proxy settings if proxies are provided
    if proxies:
        proxy_url = proxies.get("http") or proxies.get("https")
        if proxy_url:
            http = urllib3.ProxyManager(proxy_url)

    try:
        # Make a GET request
        response = http.request("GET", url, timeout=timeout / 1000)
        # Check if the request was successful (HTTP status code 200)
        return response.status == 200
    except (
        urllib3.exceptions.HTTPError,
        urllib3.exceptions.TimeoutError,
        urllib3.exceptions.MaxRetryError,
        urllib3.exceptions.ProxyError,
        urllib3.exceptions.SSLError,
    ) as e:
        return False


def test_urls_concurrently(
    urls: List[str], proxies: Mapping, timeout: int
) -> List[bool]:
    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = {
            executor.submit(url_tests, url, proxies, timeout): url for url in urls
        }
        results = []
        for future in concurrent.futures.as_completed(futures):
            try:
                success = future.result()
                results.append(success)
            except Exception as e:
                results.append(False)
        return results


def test_proxies_concurrently(proxies: tuple["ProxyConfig"]) -> List[bool]:
    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = {
            executor.submit(
                url_tests,
                proxy.test_urls[0],
                {"http": proxy.http_proxy, "https": proxy.socks_proxy},
                proxy.test_timeout,
            ): proxy
            for proxy in proxies
        }
        results = []
        for future in concurrent.futures.as_completed(futures):
            try:
                success = future.result()
                results.append(success)

            except Exception as e:
                results.append(False)

        return results


@dataclass
class ProxyConfig:
    url: str
    http_port: str
    socks_port: str
    authentication_user: str = None
    password: str = None
    alternative_urls: tuple[str] = tuple()
    label: str = "Default"

    test_urls: tuple[str] = (
        "https://www.google.com",
        "https://www.facebook.com",
        "https://www.twitter.com",
        "https://www.instagram.com",
    )
    test_timeout: int = 10_000  # Microseconds

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
