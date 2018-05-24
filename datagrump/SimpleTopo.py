import os
import sys
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import CPULimitedHost
from mininet.link import TCLink
from mininet.node import OVSController
from mininet.node import Controller
from mininet.node import RemoteController
from mininet.cli import CLI
from subprocess import Popen
from time import sleep, time
import pdb

class SimpleTopo(Topo):
    ''' dead simple topology '''
    def build(self):

            leftHost = self.addHost( 'h1' )
            rightHost = self.addHost( 'h2' )
            switch = self.addSwitch( 's1' )

            # Add links
            self.addLink( leftHost, switch )
            self.addLink( switch, rightHost )


def experiment(net):
        net.start()
        net.pingAll()
        sender, recver = net.hosts
        a = recver.popen('receiver 9090')
        b = sender.popen('sender 10.0.0.2 9090') # host needs to be $MAHIMAHI_BASE, whatever that is
        net.stop()

def main():
    topo = SimpleTopo()
    net = Mininet(topo=topo, host=CPULimitedHost, link = TCLink)
    experiment(net)

if __name__ == "__main__":
    main()