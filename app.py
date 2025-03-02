import json
import os
import datetime
from flask import Flask, request, jsonify, render_template
import pyotp
import base64
from dotenv import load_dotenv
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad, unpad
import base64

app = Flask(__name__)
load_dotenv()

# Load user secrets from .env (stored as a JSON-like string)
USER_SECRETS = json.loads(os.getenv("TOTP_SECRETS", "{}"))
MAX_VALIDATIONS = int(os.getenv("MAX_VALIDATIONS", "1"))  # Number of times a TOTP can be validated
validation_logs = []  # Store validation attempts (successful and failed)
validation_counts = {}  # Track validation count per TOTP

def decrypt_totp(key, cipher_text):
    # Ensure the key is 16, 24, or 32 bytes long
    key = key.encode('utf-8')
    key = key.ljust(32, b'\0')[:32]  # Pad or truncate the key to 32 bytes (256 bits)
    
    # Create AES cipher in ECB mode
    cipher = AES.new(key, AES.MODE_ECB)
    
    # Decode the URL-safe base64-encoded ciphertext
    cipher_text = base64.urlsafe_b64decode(cipher_text)
    
    # Decrypt the ciphertext
    plain_text = cipher.decrypt(cipher_text)
    
    # Unpad the plaintext
    plain_text = unpad(plain_text, AES.block_size)
    
    # Return the plaintext as a string
    return plain_text.decode('utf-8')

@app.route('/validate', methods=['GET'])
def validate_totp():
    """Validates the encrypted TOTP from the QR code."""
    account_id = request.args.get('account_id')
    encrypted_totp = request.args.get('totp_enc')
    timestamp = datetime.datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S')
    user_ip = request.remote_addr

    if not account_id or not encrypted_totp:
        validation_logs.append({"timestamp": timestamp, "account_id": account_id, "status": "failed", "reason": "Missing parameters", "ip": user_ip})
        return jsonify({"success": False, "error": "Missing parameters"}), 400

    # Get the user's secret key
    secret = USER_SECRETS.get(account_id)
    if not secret:
        validation_logs.append({"timestamp": timestamp, "account_id": account_id, "status": "failed", "reason": "Invalid account ID", "ip": user_ip})
        return jsonify({"success": False, "error": "Invalid account ID"}), 400

    # Decrypt the TOTP
    decrypted_totp = decrypt_totp(secret, encrypted_totp)
    if not decrypted_totp:
        validation_logs.append({"timestamp": timestamp, "account_id": account_id, "status": "failed", "reason": "Invalid TOTP encryption", "ip": user_ip})
        return jsonify({"success": False, "error": "Invalid TOTP encryption"}), 400

    # Check if the TOTP has exceeded max validations
    if decrypted_totp in validation_counts and validation_counts[decrypted_totp] >= MAX_VALIDATIONS:
        validation_logs.append({"timestamp": timestamp, "account_id": account_id, "status": "failed", "reason": "TOTP validation limit reached", "ip": user_ip})
        return jsonify({"success": False, "error": "TOTP validation limit reached"}), 400

    # Validate the TOTP
    totp = pyotp.TOTP(secret)
    if totp.verify(decrypted_totp):
        validation_counts[decrypted_totp] = validation_counts.get(decrypted_totp, 0) + 1
        validation_logs.append({"timestamp": timestamp, "account_id": account_id, "status": "success", "ip": user_ip})
        return jsonify({"success": True, "message": "TOTP is valid"}), 200
    else:
        validation_logs.append({"timestamp": timestamp, "account_id": account_id, "status": "failed", "reason": "Invalid TOTP", "ip": user_ip})
        return jsonify({"success": False, "error": "Invalid TOTP"}), 400

@app.route('/admin', methods=['GET'])
def admin_dashboard():
    """Admin dashboard to view validation logs."""
    return render_template('admin.html', validation_logs=validation_logs, max_validations=MAX_VALIDATIONS)

if __name__ == '__main__':
    #app.run(debug=True)
    app.run(host='192.168.66.185', port=5000, debug=True)
