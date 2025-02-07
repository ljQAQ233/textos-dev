net:
	if ! ip link show tap0 &> /dev/null ; then \
		sudo ip tuntap add dev tap0 mode tap ; \
		sudo ip link set tap0 up ; \
		sudo ip addr add 192.168.2.1/24 dev tap0 ; \
	fi
