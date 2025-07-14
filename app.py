import os
import tempfile
import shutil
import uuid
import random
import string
import json
from flask import Flask, request, render_template_string, redirect, url_for
from datetime import datetime, timedelta

app = Flask(__name__)

# ======= Persistent Token Store ========
USED_TOKEN_FILE = "used_tokens.json"
if os.path.exists(USED_TOKEN_FILE):
    with open(USED_TOKEN_FILE, 'r') as f:
        USED_TOKENS = set(json.load(f))
else:
    USED_TOKENS = set()

def save_used_tokens():
    with open(USED_TOKEN_FILE, 'w') as f:
        json.dump(list(USED_TOKENS), f)

def generate_unique_token():
    while True:
        token = uuid.uuid4().hex + ''.join(random.choices(string.ascii_letters + string.digits, k=8))
        if token not in USED_TOKENS:
            USED_TOKENS.add(token)
            save_used_tokens()
            return token

# ======= Link Memory Cache ========
LINKS = {}  # {token: {plist, expires}}

# ======= HTML Templates ========
UPLOAD_FORM = """
<!DOCTYPE html>
<html>
<head>
    <title>iOS Install Link Generator</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
</head>
<body class="bg-dark text-light">
<div class="container py-5">
    <h2 class="mb-4 text-warning">iOSAppSignature Link Generator</h2>
    <form method="POST" action="/upload" enctype="multipart/form-data" class="bg-secondary p-4 rounded">
        <div class="mb-3">
            <label class="form-label">IPA File</label>
            <input type="file" name="ipa" class="form-control" required>
        </div>
        <div class="mb-3">
            <label class="form-label">Bundle ID</label>
            <input name="bundle_id" class="form-control" required>
        </div>
        <div class="mb-3">
            <label class="form-label">App Title</label>
            <input name="title" class="form-control" required>
        </div>
        <div class="mb-3">
            <label class="form-label">Version</label>
            <input name="version" class="form-control" required>
        </div>
        <button class="btn btn-warning">Generate Install Link</button>
    </form>
</div>
</body>
</html>
"""

RESULT_HTML = """
<!DOCTYPE html>
<html>
<head>
    <title>Install Link Generated</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
</head>
<body class="bg-success text-white">
<div class="container py-5">
    <h2 class="mb-4">✅ Install Link Generated</h2>
    <p class="lead">Click the link below on your iOS device to install the app:</p>
    <div class="p-3 bg-dark rounded text-warning">
        <code>{{ install_link }}</code>
    </div>
    <br>
    <a href="{{ install_link }}" class="btn btn-light" target="_blank">📲 Tap to Install</a>
    <p class="mt-4 text-light">⏳ Link expires on: {{ expires }}</p>
    <a href="/" class="btn btn-outline-light mt-2">⬅️ Back</a>
</div>
</body>
</html>
"""

# ======= Routes ========
@app.route('/')
def index():
    return render_template_string(UPLOAD_FORM)

@app.route('/upload', methods=['POST'])
def upload():
    ipa = request.files['ipa']
    bundle_id = request.form['bundle_id']
    title = request.form['title']
    version = request.form['version']

    temp_dir = tempfile.mkdtemp()
    ipa_path = os.path.join(temp_dir, ipa.filename)
    ipa.save(ipa_path)

    # Simulate file use and delete immediately
    shutil.rmtree(temp_dir)

    token = generate_unique_token()
    ipa_url = f"{request.url_root}download/ipa/{token}"
    plist_url = f"{request.url_root}plist/{token}"

    plist = f'''<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
"http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict><key>items</key><array><dict>
<key>assets</key><array><dict>
<key>kind</key><string>software-package</string>
<key>url</key><string>{ipa_url}</string>
</dict></array>
<key>metadata</key><dict>
<key>bundle-identifier</key><string>{bundle_id}</string>
<key>bundle-version</key><string>{version}</string>
<key>kind</key><string>software</string>
<key>title</key><string>{title}</string>
</dict></dict></array></dict></plist>'''

    LINKS[token] = {
        'plist': plist,
        'expires': datetime.utcnow() + timedelta(days=15)
    }

    install_link = f"itms-services://?action=download-manifest&url={plist_url}"
    return render_template_string(RESULT_HTML, install_link=install_link, expires=LINKS[token]['expires'].strftime('%Y-%m-%d %H:%M:%S UTC'))

@app.route('/plist/<token>')
def serve_plist(token):
    link = LINKS.get(token)
    if not link or datetime.utcnow() > link['expires']:
        return "❌ Expired or invalid plist link", 404
    return app.response_class(link['plist'], mimetype='text/xml')

@app.route('/download/ipa/<token>')
def simulate_ipa_download(token):
    link = LINKS.get(token)
    if not link or datetime.utcnow() > link['expires']:
        return "❌ IPA expired or invalid", 404
    return "✅ IPA would be downloaded here. Replace this with S3, CDN, or real file server."

# ======= Start ========
if __name__ == '__main__':
    app.run(host="0.0.0.0", port=int(os.environ.get("PORT", 5000)))
