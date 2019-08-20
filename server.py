#!/usr/bin/python3
"""
Generic Python3 UDP server for Creator Ci40 for use with 6LoWPAN Clicker UDP client
examples.
"""
import argparse
parser = argparse.ArgumentParser()
parser.add_argument("-p", "--port", help="Port to bind to", default=3000, type=int)
parser.add_argument("-b", "--buffer", help="Buffer size", default=1024, type=int)
args = parser.parse_args()
import socket
# No IP to connect to needed for a server
IP = "::"
PORT = args.port
# Creates a socket using IPV6 and accepting datagrams
sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
sock.bind((IP, PORT))
print("Server initialised, awaiting data. Use Ctrl + C to stop")
while True:
	data, address = sock.recvfrom(args.buffer)
	# Print client data as it arrives. For unpacking the data or converting to a UTF-8 string see below:
	# https://docs.python.org/3.5/library/struct.html
	# https://docs.python.org/3/howto/unicode.html#the-string-type
	print(data)