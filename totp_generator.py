import pyotp
import qrcode
import os
import base64
import json
from dotenv import load_dotenv
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend

# Load environment variables
load_dotenv()

# Load user secrets
USER_SECRETS = json.loads(os.getenv("TOTP_SECRETS", "{}"))

def encrypt_totp(secret_key: str, totp_code: str) -> str:
    """Encrypts the TOTP code using AES-GCM with the secret key."""
    key = secret_key[:32].ljust(32, '0').encode()  # Ensure key is 32 bytes
    iv = os.urandom(12)  # Generate a random IV (Nonce) for AES-GCM

    cipher = Cipher(algorithms.AES(key), modes.GCM(iv), backend=default_backend())
    encryptor = cipher.encryptor()

    ciphertext = encryptor.update(totp_code.encode()) + encryptor.finalize()
    encrypted_totp = base64.urlsafe_b64encode(iv + encryptor.tag + ciphertext).decode()

    return encrypted_totp

def generate_totp_qr(account_id: str):
    """Generates a QR code with an encrypted TOTP code for a specific user."""
    secret = USER_SECRETS.get(account_id)
    if not secret:
        raise ValueError(f"TOTP secret not found for user: {account_id}")

    # Generate the TOTP code
    totp = pyotp.TOTP(secret)
    current_totp = totp.now()

    # Encrypt the TOTP using the secret key
    encrypted_totp = encrypt_totp(secret, current_totp)

    # Create a validation URL with encrypted TOTP and account ID
    validation_url = f"http://127.0.0.1:5000/validate?account_id={account_id}&totp_enc={encrypted_totp}"

    # Print URL for testing
    print(validation_url)
    
    # Generate the QR code
    qr = qrcode.QRCode(
        version=1,
        error_correction=qrcode.constants.ERROR_CORRECT_L,
        box_size=10,
        border=4,
    )
    qr.add_data(validation_url)
    qr.make(fit=True)

    # Save the QR code image
    img = qr.make_image(fill_color="black", back_color="white")
    img_path = f"static/{account_id}_totp_qr.png"
    img.save(img_path)
    print(f"QR code saved as '{img_path}'")

if __name__ == "__main__":
    # Example: Generate QR codes for Alice and Bob
    generate_totp_qr("alice123")
    generate_totp_qr("bob456")
