import pyotp
import qrcode
import os
import base64
import json
from dotenv import load_dotenv
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad, unpad
import base64
# Load environment variables
load_dotenv()

# Load user secrets
USER_SECRETS = json.loads(os.getenv("TOTP_SECRETS", "{}"))

def encrypt_totp(key, plain_text):
    # Ensure the key is 16, 24, or 32 bytes long
    key = key.encode('utf-8')
    key = key.ljust(32, b'\0')[:32]  # Pad or truncate the key to 32 bytes (256 bits)
    
    # Create AES cipher in ECB mode
    cipher = AES.new(key, AES.MODE_ECB)
    
    # Pad the plaintext to be a multiple of 16 bytes
    plain_text = pad(plain_text.encode('utf-8'), AES.block_size)
    
    # Encrypt the plaintext
    cipher_text = cipher.encrypt(plain_text)
    
    # Return the ciphertext as a URL-safe base64-encoded string
    return base64.urlsafe_b64encode(cipher_text).decode('utf-8')

def generate_totp_qr(account_id: str):
    """Generates a QR code with an encrypted TOTP code for a specific user."""
    secret = USER_SECRETS.get(account_id)
    if not secret:
        raise ValueError(f"TOTP secret not found for user: {account_id}")

    # Generate the TOTP code
    totp = pyotp.TOTP(secret)
    current_totp = totp.now()
    print(current_totp)
    # Encrypt the TOTP using the secret key
    encrypted_totp = encrypt_totp(secret, current_totp)

    # Create a validation URL with encrypted TOTP and account ID
    validation_url = f"http://192.168.66.185:5000/validate?account_id={account_id}&totp_enc={encrypted_totp}"

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
