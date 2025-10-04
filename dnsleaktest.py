#!/usr/bin/env python3
# encoding=utf-8
# Any questions: tutumbul@gmail.com
# https://bash.ws/dnsleak

# https://raw.githubusercontent.com/macvk/dnsleaktest/master/dnsleaktest.py

import os
import subprocess
import json
from random import randint
from platform import system as system_name
from subprocess import call as system_call


from urllib.request import urlopen

from color_escape import GREEN, RED, YELLOW, BOLD, RESET

def ping(host):
    fn = open(os.devnull, 'w')
    param = '-n' if system_name().lower() == 'windows' else '-c'
    command = ['ping', param, '1', host]
    retcode = system_call(command, stdout=fn, stderr=subprocess.STDOUT)
    fn.close()
    return retcode == 0


class DNSLeakTester:
    """
    A class to perform DNS leakage tests using the bash.ws service.
    
    Usage:
    ------
    dns_tester = DNSLeakTester()
    dns_tester.perform_test()
    """

    def __init__(self):
        self.leak_id = None
        self.test_results = []

    def fetch_leak_id(self):
        """
        Fetch a unique leak ID from the bash.ws service to track DNS requests.
        """
        print('Fetching leak ID...')
        try:
            response = urlopen("https://bash.ws/id")
            self.leak_id = response.read().decode("utf-8")
        except Exception as e:
            print(f"Error fetching leak ID: {e}")
            self.leak_id = None
        
        print(f'Leak ID: {YELLOW}{self.leak_id}{RESET}')

    def simulate_dns_requests(self):
        """
        Simulate DNS requests to test for leaks by pinging custom domains.
        """
        
        if not self.leak_id:
            raise RuntimeError("Leak ID not available. Cannot perform DNS simulation.")
            
        
        print('Simulating DNS requests...')

        for x in range(10):
            domain = f"{x}.{self.leak_id}.bash.ws"
            self.ping_domain(domain)

    def ping_domain(self, domain: str):
        """
        Simulate a DNS request by 'pinging' a domain. This method is a placeholder 
        and should be implemented to actually perform DNS requests.
        
        :param domain: The domain to ping.
        """
        # Implement actual DNS request logic here.
        return ping(domain)

    def retrieve_test_results(self):
        """
        Retrieve and parse test results from the bash.ws service.
        """
        
        print('Retrieving test results...')

        try:
            response = urlopen(f"https://bash.ws/dnsleak/test/{self.leak_id}?json")
            data = response.read().decode("utf-8")
            self.test_results = json.loads(data)
        except Exception as e:
            print(f"Error retrieving test results: {e}")
            self.test_results = []

    def display_results(self):
        """
        Display the results of the DNS leak test, including IP addresses and DNS servers used.
        """
        self.display_ip_info()
        dns_servers = self.get_dns_servers()

        if not dns_servers:
            print("No DNS servers found")
        else:
            print(f"You use {BOLD}{len(dns_servers)}{RESET} DNS servers:")
            for dns_server in dns_servers:
                print(self.format_server_info(dns_server))

        self.display_conclusion()

    def display_ip_info(self):
        """
        Display IP address information based on the test results.
        """
        ip_info = next((server for server in self.test_results if server['type'] == "ip"), None)
        if ip_info:
            print("Your IP:")
            print(f"{GREEN}{self.format_server_info(ip_info)}{RESET}")

    def get_dns_servers(self):
        """
        Extract DNS servers from the test results.

        :return: A list of DNS servers.
        """
        return [server for server in self.test_results if server['type'] == "dns"]

    def display_conclusion(self):
        """
        Display the conclusion of the DNS leak test.
        """
        conclusion_info = next((server for server in self.test_results if server['type'] == "conclusion"), None)
        if conclusion_info and conclusion_info.get('ip'):
            print(f"{BOLD}{YELLOW}Conclusion:{RESET}")
            print(conclusion_info['ip'])

    def format_server_info(self, server):
        """
        Format server information into a readable string.

        :param server: The server dictionary with IP, country, and ASN info.
        :return: A formatted string representing the server information.
        """
        country_info = f" [{server['country_name']}]" if server['country_name'] else ""
        asn_info = f", {server['asn']}" if server.get('asn') else ""
        return f"{server['ip']}{country_info}{asn_info}"

    def perform_test(self):
        """
        Perform a full DNS leak test, including fetching the leak ID, simulating DNS requests,
        retrieving the results, and displaying the findings.
        """
        self.fetch_leak_id()
        self.simulate_dns_requests()
        self.retrieve_test_results()
        self.display_results()
