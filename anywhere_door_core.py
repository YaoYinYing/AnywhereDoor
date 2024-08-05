import os
import sys
from typing import Union

from datastructures import ProxyConfig, ProxyTable, ProxyType, test_proxies_concurrently, test_urls_concurrently
from predefined_proxies import predefined_proxies,NoProxy


# Color escape sequences
GREEN = "\033[0;32m"
RED = "\033[0;31m"
YELLOW = "\033[0;33m"
RESET = "\033[0m"
BOLD= "\033[1m"


class AnywhereDoor:
    predefined_proxy_table: ProxyTable = ProxyTable(proxies=predefined_proxies)

    def __init__(self):
        self.predefined_proxies = self.predefined_proxy_table.expanded
        self.in_use_proxy = self.predefined_proxies[0]

    @property
    def anywhere_door_open(self) -> bool:
        return all(
            os.environ.get(p_kw) for p_kw in ["http_proxy", "https_proxy", "all_proxy"]
        )

    def activate_anywhere_door(self):
        if self.in_use_proxy:
            print(f'export https_proxy="{self.in_use_proxy.http_proxy}";')
            print(f'export http_proxy="{self.in_use_proxy.http_proxy}";')
            print(f'export all_proxy="{self.in_use_proxy.socks_proxy}";')
            print(f'export NO_PROXY="{str(NoProxy())}";')

            print(f"echo '{GREEN}proxy selected: {RESET} {YELLOW} {self.in_use_proxy.label} {RESET}';")
            print(f"echo '{YELLOW}{str(self.in_use_proxy)}{RESET}';")

        else:
            print(f"echo '{YELLOW}No proxy selected.{RESET}';")

    def deactivate_anywhere_door(self):
        print("unset https_proxy;")
        print("unset http_proxy;")
        print("unset all_proxy;")
        print(f"echo '{GREEN}Anywhere Door is now inactive!{RESET}';")

    def configure_anywhere_door(
        self, new_ip, new_port, new_socks_port, authentication_user=None, password=None
    ):
        if not all((new_ip, new_port, new_socks_port)):
            print(f"echo '{RED}Please provide valid IP and port values.{RESET}';")
            print(
                "echo 'Usage: anywhere_door config ip port socks_port [authentication_user] [password]';"
            )
            return
        self.in_use_proxy = ProxyConfig(
            new_ip, new_port, new_socks_port, authentication_user, password, label='Configured'
        )
        self.activate_anywhere_door()

    def show_configurations(self, protocol: ProxyType = None):
        if not self.anywhere_door_open:
            print(
                f"echo '{YELLOW}Anywhere Door is inactive. No configurations to show.{RESET}';"
            )
            return
        protocols = {
            "http": os.environ.get("http_proxy", ""),
            "https": os.environ.get("https_proxy", ""),
            "all": os.environ.get("all_proxy", ""),
        }

        if not (protocol and protocol in protocols):
            for pk,pv in protocols.items():
                print(f"echo '${pk}_proxy={GREEN}{pv}{RESET}';")
            return

        print(f"echo '${protocol}_proxy={protocols[protocol]}';")

    def configure_git_proxy(self):
        if self.anywhere_door_open:
            print(f"echo 'Enabling proxy for git';")
            git_command = (
                f"git config --global http.proxy {self.in_use_proxy.http_proxy};"
            )
            print(git_command)

            git_command = (
                f"git config --global https.proxy {self.in_use_proxy.http_proxy};"
            )
            print(git_command)
            print(f"echo 'Done';")
        else:
            print(f"echo 'Disabling proxy for git';")
            git_command = "git config --global --unset http.proxy;"
            print(git_command)

            git_command = "git config --global --unset https.proxy;"
            print(git_command)
            print(f"echo 'Done';")

    @property
    def system_proxy(self) -> tuple[str]:
        http_proxy = os.environ.get("http_proxy", "")
        https_proxy = os.environ.get("https_proxy", "")
        all_proxy = os.environ.get("all_proxy", "")

        return (http_proxy, https_proxy, all_proxy)

    @staticmethod
    def match_proxy(proxy: ProxyConfig, proxy_tuple):
        return all(
            p1 == p2
            for p1, p2 in zip(
                [proxy.http_proxy, proxy.http_proxy, proxy.socks_proxy], proxy_tuple
            )
        )

    def use_proxy(self, show_raw=False, index: Union[str, int]=None):
        system_proxy = self.system_proxy
        if index is None or show_raw:
            print("echo 'Available proxies:';")
            print("echo '" + "-" * 45 + "';")
            for i, proxy in enumerate(
                self.predefined_proxies
                if not show_raw
                else self.predefined_proxy_table.proxies,
                start=1,
            ):
                is_system_proxy = self.match_proxy(proxy, system_proxy)
                print(
                    f"echo '{GREEN if is_system_proxy else ''}{i}. {BOLD}[{proxy.label}]{RESET} {str(proxy)} {RESET if is_system_proxy else ''}';"
                )
            print("echo '" + "-" * 45 + "';")

            if not show_raw:
                print(
                    f"echo 'Please use `anywhere_door use {RED}<index>{RESET}` to pick one of them.';"
                )
        else:
            try:
                if isinstance(index, int) or (isinstance(index, str) and index.isdigit()):
                    index = int(index)
                    self.in_use_proxy = self.predefined_proxies[index - 1]
                    return self.activate_anywhere_door()
                if isinstance(index, str) and index in [x.label for x in self.predefined_proxies]:
                    self.in_use_proxy = [x for x in self.predefined_proxies if x.label == index][0]
                    return self.activate_anywhere_door()

                raise ValueError
                
            except IndexError or ValueError:
                print(f"echo '{RED}Invalid proxy index:{index}.{RESET}';")
    def show_help(self, command: str = None):
        if not command:
            print("Anywhere Door: A quick switch for network proxies in the current session.")
            print("Usage: anywhere_door [command]")
            print("Commands:")
            print("   on        : Activate Anywhere Door")
            print("   off       : Deactivate Anywhere Door")
            print("   config    : Configure custom IP and port")
            print("   show      : Show the current proxy configurations")
            print("   list/ls   : Show all predefined proxies")
            print("   test      : Perform a test connection to check proxy accessibility")
            print("   bench     : Perform a speed test connection to check network bandwidth")
            print("   wget      : Perform a speed test connection via wget to check network bandwidth")
            print("   curl      : Perform a speed test connection via curl to check network bandwidth")
            print("   whereami  : Perform a location check via curl")
            print("   use       : Use a specific proxy from the configured list")
            print("   upgrade   : Upgrade to the latest code")
            print("   dns/leak  : DNS Leak Test")
            print("   help/?    : Show this help message and exit")
            return

        if command == "test":
            print("Testing proxies.")
            print("Usage: anywhere_door test [opt]")
            print("   <empty>   : Test current proxy.")
            print("   all       : Test all predefined proxies, simplified results.")
            print("   full      : Test all predefined proxies, detailed results.")
            return

        if command == "use":
            print("Call a predefined proxy.")
            print("Usage: anywhere_door use [opt]")
            print("   <empty>   : Show all predefined proxies.")
            print("   [index]   : Set indexed proxy.")
            print("   [label]   : Set label proxy.")
            return

        if command == "config":
            print("Configure a new proxy.")
            print("Usage: anywhere_door config server_url http_port socks_port [username] [password]")
            return

        if command == "show":
            print("Show current proxy.")
            print("Usage: anywhere_door show [proxy_type: 'http', 'https', 'all']")
            return

        print(f"No such help message for command [{command}].")




def anywhere_door(command, *args):
    door = AnywhereDoor()
    if command == "on" or command == "":
        return door.activate_anywhere_door()
    if command == "off":
         return door.deactivate_anywhere_door()
    if command == "config":
         return door.configure_anywhere_door(*args)
    if command == "help" or command == "?":
        try:
            opt = str(args[0])
            return door.show_help(command=opt)
        except ValueError:
             return door.show_help()
    if command == "show":
        try:
            protocol = str(args[0])
            return door.show_configurations(protocol=protocol)
        except ValueError:
            return door.show_configurations()
    if command == "git":
        return door.configure_git_proxy()
    if command == "list" or command == "ls":
        return door.use_proxy(show_raw=True)
    
    if command == "dns" or command == "leak":
        from dnsleaktest import dns_leakage_test
        return dns_leakage_test()
    if command == "use":
        if args[0] == '':
            return door.use_proxy()
        try:
            index = str(args[0])
            return door.use_proxy(index=index)
        except ValueError as e:
            print(f'echo "Invalid proxy index {e}.";')
            return door.use_proxy()
            

    if command == "test":      
        if args[0] == '':
            proxies = {
        "http": door.system_proxy[0],
        "https": door.system_proxy[1],
    }
            p=ProxyConfig(None,None, None)
            res=test_urls_concurrently(p.test_urls, proxies, p.test_timeout)
            for u,_res in zip(p.test_urls,res):
                if _res:
                    print(f"Connection test to {GREEN}{u}: Success{RESET}")
                else:
                    print(f"Connection test to {RED}{u}: Failed{RESET}")

            return
            
        if  args[0] == "full":
            all_available_proxies: list[int] =[]
            print('''
===========================================================================
URL Testing ... ...
===========================================================================''')
            
            res=test_proxies_concurrently(door.predefined_proxies)
            for (i, proxy), (j,_res) in zip(enumerate(
                    door.predefined_proxies,
                    start=1,
                ), enumerate(res, start=1)):
                print(f'Testing: {i}: {GREEN}{proxy.label}{RESET}')
                print(f'{YELLOW}{str(proxy)}{RESET}')
                if _res:
                    print(f'Connection test of {GREEN}{proxy.test_urls[0]}: Success{RESET}')
                    all_available_proxies.append(i)
                else:
                    print(f'Connection test of {RED}{proxy.test_urls[0]}: Failed{RESET}')
                print('-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-')
            
            print("===========================================================================")
            print(f'Testing Passed: {GREEN}{" ".join(tuple(str(i) for i in all_available_proxies))}{RESET}')
            print("===========================================================================")
            return 

        if  args[0] == "all":
            print(f'Testing ', end='', flush=True)
            res=test_proxies_concurrently(door.predefined_proxies)
            for (i, proxy), (j,_res) in zip(enumerate(
                    door.predefined_proxies,
                    start=1,
                ), enumerate(res, start=1)):
                print(f' {GREEN if _res else RED}{i}{RESET}', end='', flush=True)

            print('')
            return 
    else:
        print(f"echo -e 'Unknown command: {RED}{command}{RESET}'")



if __name__ == "__main__":
    if len(sys.argv[1:]) >= 1:
        anywhere_door(*sys.argv[1:])
    else:
        anywhere_door(command="on")
