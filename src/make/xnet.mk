IFACE:=wlan0
GATEWAY:=192.168.2.1

net:
	if ! ip link show tap0 &> /dev/null ; then \
		sudo ip tuntap add dev tap0 mode tap ; \
		sudo ip link set tap0 up ; \
		sudo ip addr add $(GATEWAY)/24 dev tap0 ; \
		sudo sysctl -w net.ipv4.ip_forward=1 ; \
		sudo iptables -t nat -A POSTROUTING -s $(GATEWAY)/24 -o $(IFACE) -j MASQUERADE ; \
		sudo iptables -A FORWARD -i tap0 -o $(IFACE) -j ACCEPT ; \
		sudo iptables -A FORWARD -i $(IFACE) -o tap0 -m state --state RELATED,ESTABLISHED -j ACCEPT ; \
	fi
