import sys
import os
import smtplib
from email.message import EmailMessage

def getenv_any(*names):
    for e in names:
        t = os.getenv(e)
        if t is not None:
            return t
    print("Error: environment variable " + names[0] + " was not set")
    exit(1)

zs_from = getenv_any("PDC_SMTP_FROM", "ZANO_SMTP_FROM")
zs_addr = getenv_any("PDC_SMTP_ADDR", "ZANO_SMTP_ADDR")
zs_port = getenv_any("PDC_SMTP_PORT", "ZANO_SMTP_PORT")
zs_user = getenv_any("PDC_SMTP_USER", "ZANO_SMTP_USER")
zs_pass = getenv_any("PDC_SMTP_PASS", "ZANO_SMTP_PASS")

if len(sys.argv) != 4:
    print("Usage: " + sys.argv[0] + " <subject> <recipient(s)> <body>")
    exit(1)

msg = EmailMessage()
msg['Subject'] = sys.argv[1]
msg['From'] = zs_from
msg['To'] = sys.argv[2]
msg.add_header('Content-Type','text/html')
msg.set_payload(sys.argv[3])

s = smtplib.SMTP(zs_addr, zs_port)
s.starttls()
s.login(zs_user, zs_pass)
s.send_message(msg)
s.quit()

print("e-mail sent.")
