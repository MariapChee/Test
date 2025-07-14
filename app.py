import os
import tempfile
import shutil
import uuid
import random
import string
import json
import zipfile
import plistlib
import base64
import requests
from flask import Flask, request, jsonify, render_template_string

app = Flask(__name__)
app.config['MAX_CONTENT_LENGTH'] = 2 * 1024 * 1024 * 1024  # 2GB

# GitHub Config
GITHUB_TOKEN = "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"
GITHUB_REPO = "MariapChee/Test"
GITHUB_BRANCH = "main"

# Used tokens tracking
USED_TOKEN_FILE = "used_tokens.json"
if os.path.exists(USED_TOKEN_FILE):
    with open(USED_TOKEN_FILE, "r") as f:
        USED_TOKENS = set(json.load(f))
else:
    USED_TOKENS = set()

def save_used_tokens():
    with open(USED_TOKEN_FILE, "w") as f:
        json.dump(list(USED_TOKENS), f)

def generate_unique_token():
    while True:
        token = uuid.uuid4().hex + ''.join(random.choices(string.ascii_letters + string.digits, k=6))
        if token not in USED_TOKENS:
            USED_TOKENS.add(token)
            save_used_tokens()
            return token

def extract_info_from_ipa(ipa_path):
    with tempfile.TemporaryDirectory() as tmpdir:
        with zipfile.ZipFile(ipa_path, 'r') as zip_ref:
            zip_ref.extractall(tmpdir)
        payload_path = os.path.join(tmpdir, "Payload")
        apps = [d for d in os.listdir(payload_path) if d.endswith(".app")]
        if not apps:
            raise Exception("No .app folder found")
        plist_path = os.path.join(payload_path, apps[0], "Info.plist")
        with open(plist_path, "rb") as f:
            plist = plistlib.load(f)
        return {
            "bundle_id": plist.get("CFBundleIdentifier", "com.example.app"),
            "version": plist.get("CFBundleVersion", "1.0"),
            "title": plist.get("CFBundleDisplayName") or plist.get("CFBundleName") or "MyApp"
        }

def upload_to_github(file_path, github_path):
    with open(file_path, "rb") as f:
        content = base64.b64encode(f.read()).decode()
    url = f"https://api.github.com/repos/{GITHUB_REPO}/contents/{github_path}"
    headers = {
        "Authorization": f"Bearer {GITHUB_TOKEN}",
        "Accept": "application/vnd.github+json"
    }
    data = {
        "message": f"Upload {github_path}",
        "content": content,
        "branch": GITHUB_BRANCH
    }
    r = requests.put(url, headers=headers, json=data)
    if r.status_code in [200, 201]:
        return f"https://raw.githubusercontent.com/{GITHUB_REPO}/{GITHUB_BRANCH}/{github_path}"
    else:
        raise Exception(f"GitHub upload failed: {r.json().get('message')}")

@app.route('/')
def index():
    return render_template_string(open("index.html").read())

@app.route('/upload', methods=['POST'])
def upload():
    if 'ipa' not in request.files:
        return jsonify({"error": "IPA missing"}), 400

    ipa = request.files['ipa']
    temp_dir = tempfile.mkdtemp()
    ipa_path = os.path.join(temp_dir, ipa.filename)
    ipa.save(ipa_path)

    try:
        info = extract_info_from_ipa(ipa_path)
    except Exception as e:
        shutil.rmtree(temp_dir)
        return f"<h3 style='color:red'>Failed to extract Info.plist: {str(e)}</h3>"

    token = generate_unique_token()
    ipa_filename = f"{token}.ipa"
    plist_filename = f"{token}.plist"

    try:
        ipa_url = upload_to_github(ipa_path, ipa_filename)
    except Exception as e:
        shutil.rmtree(temp_dir)
        return f"<h3 style='color:red'>IPA upload failed: {str(e)}</h3>"

    plist_content = f'''<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
"http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict><key>items</key><array><dict>
<key>assets</key><array><dict>
<key>kind</key><string>software-package</string>
<key>url</key><string>{ipa_url}</string>
</dict></array>
<key>metadata</key><dict>
<key>bundle-identifier</key><string>{info['bundle_id']}</string>
<key>bundle-version</key><string>{info['version']}</string>
<key>kind</key><string>software</string>
<key>title</key><string>{info['title']}</string>
</dict></dict></array></dict></plist>'''

    plist_path = os.path.join(temp_dir, plist_filename)
    with open(plist_path, "w") as f:
        f.write(plist_content)

    try:
        plist_url = upload_to_github(plist_path, plist_filename)
    except Exception as e:
        shutil.rmtree(temp_dir)
        return f"<h3 style='color:red'>Plist upload failed: {str(e)}</h3>"

    shutil.rmtree(temp_dir)
    install_link = f"itms-services://?action=download-manifest&url={plist_url}"

    if request.headers.get("Accept") == "application/json":
        return jsonify({
            "install_link": install_link,
            "ipa_url": ipa_url,
            "plist_url": plist_url
        })

    return f"""
    <html><body style='background:#0d1117;color:white;font-family:sans-serif;text-align:center;padding:50px'>
    <h1>Link Ready</h1>
    <p>Tap the link below on iPhone/iPad:</p>
    <code style='background:#111;padding:10px;border-radius:5px;display:block;margin-bottom:10px'>{install_link}</code>
    <a href="{install_link}" style="color:#0f0;font-size:20px;">Install App</a><br><br>
    <a href="/" style="color:#ccc;">Upload Another</a>
    </body></html>
    """

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=int(os.environ.get("PORT", 5000)))
