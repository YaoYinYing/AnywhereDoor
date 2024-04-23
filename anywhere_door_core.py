import os
import sys
from datastructures import ProxyConfig, ProxyTable
from predefined_proxies import predefined_proxies


# Color escape sequences
GREEN = "\033[0;32m"
RED = "\033[0;31m"
YELLOW = "\033[0;33m"
RESET = "\033[0m"


class AnywhereDoor:
    predefined_proxy_table: ProxyTable = ProxyTable(proxies=predefined_proxies)

    def __init__(self):
        self.predefined_proxies = self.predefined_proxy_table.expanded
        self.in_use_proxy = self.predefined_proxies[0]

    @property
    def anywhere_door_open(self)-> bool:
        return all(
            os.environ.get(p_kw) for p_kw in ["http_proxy", "https_proxy", "all_proxy"]
        )

    def activate_anywhere_door(self):
        if self.in_use_proxy:
            print(f'export https_proxy="{self.in_use_proxy.http_proxy}";')
            print(f'export http_proxy="{self.in_use_proxy.http_proxy}";')
            print(f'export all_proxy="{self.in_use_proxy.socks_proxy}";')

            print(f"echo '{GREEN}proxy selected.{RESET}';")
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
        if not all((new_ip, new_port, new_socks_port)) :
            print(f"echo '{RED}Please provide valid IP and port values.{RESET}';")
            print(
                "echo 'Usage: anywhere_door config ip port socks_port [authentication_user] [password]';"
            )
            return
        self.in_use_proxy = ProxyConfig(
            new_ip, new_port, new_socks_port, authentication_user, password
        )
        self.activate_anywhere_door()

    def show_configurations(self):
        if self.anywhere_door_open:
            print(
                f"echo '$http_proxy={GREEN}{os.environ.get('http_proxy', '')}{RESET}';"
            )
            print(
                f"echo '$https_proxy={GREEN}{os.environ.get('https_proxy', '')}{RESET}';"
            )
            print(
                f"echo '$all_proxy={GREEN}{os.environ.get('all_proxy', '')}{RESET}';"
            )
        else:
            print(
                f"echo '{YELLOW}Anywhere Door is inactive. No configurations to show.{RESET}';"
            )

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

    def use_proxy(self, index=None):
        system_proxy = self.system_proxy
        if index is None:
            print("echo 'Available proxies:';")
            print("echo '" + "-" * 45 + "';")
            for i, proxy in enumerate(self.predefined_proxies, start=1):
                is_system_proxy = self.match_proxy(proxy, system_proxy)
                print(
                    f"echo '{GREEN if is_system_proxy else ''}{i}. {str(proxy)} {RESET if is_system_proxy else ''}';"
                )
            print("echo '" + "-" * 45 + "';")
            print(
                f"echo 'Please use `anywhere_door use {RED}<index>{RESET}` to pick one of them.';"
            )
        else:
            try:
                self.in_use_proxy = self.predefined_proxies[index - 1]
                self.activate_anywhere_door()
            except IndexError:
                print(f"echo '{RED}Invalid proxy index.{RESET}';")

    def show_help(self, command:str=None):
        if not command:
            print(
                "echo 'Anywhere Door: A quick switch for network proxies in the current session.';"
            )
            print("echo -e 'Usage: anywhere_door [command]';")
            print("echo -e 'Commands:';")
            print("echo -e '   on        : Activate Anywhere Door';")
            print("echo -e '   off       : Deactivate Anywhere Door';")
            print("echo -e '   config    : Configure custom IP and port';")
            print("echo -e '   show      : Show the current proxy configurations';")
            print("echo -e '   test      : Perform a test connection to check proxy accessibility';"
            )
            print("echo -e '   bench     : Perform a speed test connection to check network bandwith';"
            )
            print("echo -e '   use       : Use a specific proxy from the configured list';"
            )
            print("echo -e '   help/?    : Show this help message and exit';")
            return
        if command  == 'test':
            print("echo 'Testing proxies. ';")
            print("echo -e 'Usage: anywhere_door test [opt]';")
            print("echo -e '   <empty>   : Test current proxy.';")
            print("echo -e '   all       : Test all predefined proxies.';")
            return
        if command  == 'use':
            print("echo 'Call a predefined proxy. ';")
            print("echo -e 'Usage: anywhere_door use [opt]';")
            print("echo -e '   <empty>   : Show all predefined proxies.';")
            print("echo -e '   [index]   : Set indexed proxy.';")
            return    
        
        if command  == 'config':
            print("echo 'Config a new proxy. ';")
            print("echo -e 'Usage: anywhere_door config server_url http_port socks_port [username] [password]';")
            return 
        
        print(f"echo -e 'No such help message for command [{command}]. ';")


def anywhere_door(command, *args):
    door = AnywhereDoor()
    if command == "on" or command == "":
        door.activate_anywhere_door()
    elif command == "off":
        door.deactivate_anywhere_door()
    elif command == "config":
        door.configure_anywhere_door(*args)
    elif command == "help" or command == "?":
        try:
            command=str(args[0])
            door.show_help(command=command)
        except ValueError:
            door.show_help()
    elif command == "show":
        door.show_configurations()
    elif command == "git":
        door.configure_git_proxy()
    elif command == "use":
        if args:
            try:
                index = int(args[0])
                door.use_proxy(index)
            except ValueError:
                door.use_proxy()

    else:
        print(f"echo 'Unknown command: {command}';")


if __name__ == "__main__":
    if len(sys.argv[1:]) >= 1:
        anywhere_door(*sys.argv[1:])
    else:
        anywhere_door(command="on")
