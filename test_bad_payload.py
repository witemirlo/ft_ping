#! /usr/bin/python3

import os
import signal
import sys
from scapy.all import *

# 🚫 Desactiva respuestas ICMP del sistema
def disable_kernel_icmp_response():
    print("[*] Desactivando respuestas ICMP del sistema operativo...")
    os.system("sysctl -w net.ipv4.icmp_echo_ignore_all=1")

# ✅ Restaura el comportamiento normal del sistema
def enable_kernel_icmp_response():
    print("[*] Restaurando respuestas ICMP del sistema operativo...")
    os.system("sysctl -w net.ipv4.icmp_echo_ignore_all=0")

# 🧨 Lógica para responder con paquetes manipulados
def icmp_malformed_response(pkt):
    if pkt.haslayer(ICMP) and pkt[ICMP].type == 8:  # Echo Request
        ip = IP(dst=pkt[IP].src, src=pkt[IP].dst)
        icmp = ICMP(type=0, id=pkt[ICMP].id, seq=pkt[ICMP].seq)
        bad_payload = b"\xde\xad\xbe\xefINVALID_PAYLOAD"
        respuesta = ip / icmp / Raw(bad_payload)
        del respuesta[IP].chksum
        respuesta[ICMP].chksum = 0x1234  # Checksum incorrecto
        print(f"[+] Respondiendo a {pkt[IP].src} con datos manipulados")
        send(respuesta, verbose=False)

# 🛑 Captura Ctrl+C para restaurar el sistema
def signal_handler(sig, frame):
    print("\n[!] Interrupción detectada. Restaurando estado del sistema...")
    enable_kernel_icmp_response()
    sys.exit(0)

if __name__ == "__main__":
    # Configura el manejador de señales
    signal.signal(signal.SIGINT, signal_handler)

    disable_kernel_icmp_response()

    print("[*] Escuchando ICMP Echo Requests (Ctrl+C para salir)...")
    sniff(filter="icmp", prn=icmp_malformed_response)
