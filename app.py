import os, json, tempfile, shutil, uuid, zipfile, plistlib, base64, requests, qrcode, threading, time
from datetime import datetime, timedelta
from flask import Flask, request, Response

app = Flask(__name__)
app.config['MAX_CONTENT_LENGTH'] = 2 * 1024 * 1024 * 1024  # 2GB

# GitHub Setup
GITHUB_TOKEN = "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"
GITHUB_REPO = "MariapChee/Test"
GITHUB_BRANCH = "main"

RECORD_PATH = "used_links.json"

def save_token_timestamp(token):
    if os.path.exists(RECORD_PATH):
        with open(RECORD_PATH, "r") as f:
            data = json.load(f)
    else:
        data = {}

    data[token] = datetime.utcnow().isoformat()
    with open(RECORD_PATH, "w") as f:
        json.dump(data, f, indent=2)

def generate_token():
    return uuid.uuid4().hex[:16]

def extract_info_from_ipa(ipa_path):
    with tempfile.TemporaryDirectory() as tmpdir:
        with zipfile.ZipFile(ipa_path, 'r') as zip_ref:
            zip_ref.extractall(tmpdir)
        payload_path = os.path.join(tmpdir, "Payload")
        apps = [d for d in os.listdir(payload_path) if d.endswith(".app")]
        if not apps:
            raise Exception("No .app found in Payload")
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
    check = requests.get(url, headers=headers)
    if check.status_code == 200:
        sha = check.json()["sha"]
        data = {
            "message": f"Update {github_path}",
            "content": content,
            "branch": GITHUB_BRANCH,
            "sha": sha
        }
    else:
        data = {
            "message": f"Add {github_path}",
            "content": content,
            "branch": GITHUB_BRANCH
        }
    r = requests.put(url, headers=headers, json=data)
    if r.status_code not in [200, 201]:
        raise Exception(f"GitHub upload failed: {r.text}")
    return f"https://raw.githubusercontent.com/{GITHUB_REPO}/{GITHUB_BRANCH}/{github_path}"

def delete_from_github(github_path):
    url = f"https://api.github.com/repos/{GITHUB_REPO}/contents/{github_path}"
    headers = {
        "Authorization": f"Bearer {GITHUB_TOKEN}",
        "Accept": "application/vnd.github+json"
    }
    r = requests.get(url, headers=headers)
    if r.status_code == 200:
        sha = r.json()['sha']
        requests.delete(url, headers=headers, json={
            "message": f"Delete {github_path}",
            "sha": sha,
            "branch": GITHUB_BRANCH
        })

def cleanup_old_links():
    while True:
        if os.path.exists(RECORD_PATH):
            with open(RECORD_PATH, "r") as f:
                data = json.load(f)
        else:
            data = {}

        now = datetime.utcnow()
        expired = []

        for token, timestamp in data.items():
            try:
                t = datetime.fromisoformat(timestamp)
                if now - t > timedelta(days=14):
                    delete_from_github(f"{token}.ipa")
                    delete_from_github(f"{token}.plist")
                    qr_path = f"static/qr_{token}.png"
                    if os.path.exists(qr_path):
                        os.remove(qr_path)
                    expired.append(token)
            except:
                continue

        for token in expired:
            del data[token]

        with open(RECORD_PATH, "w") as f:
            json.dump(data, f, indent=2)

        time.sleep(86400)  # Wait 24 hours before next cleanup

threading.Thread(target=cleanup_old_links, daemon=True).start()

@app.route('/')
def index():
    return Response('''
    <!DOCTYPE html>
    <html>
    <head>
      <title>IPA Upload</title>
      <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
      <style>
        body { background: #0d1117; color: white; font-family: sans-serif; }
        .box { max-width: 600px; margin: 80px auto; background: #161b22; padding: 30px; border-radius: 12px; }
      </style>
    </head>
    <body>
      <div class="box">
        <h3 class="text-center">Upload IPA File</h3>
        <form method="POST" action="/upload" enctype="multipart/form-data">
          <input type="file" name="ipa" class="form-control my-3" required accept=".ipa">
          <button type="submit" class="btn btn-primary w-100">Generate Install Link</button>
        </form>
      </div>
    </body>
    </html>
    ''', mimetype='text/html')

@app.route('/upload', methods=['POST'])
def upload():
    if 'ipa' not in request.files:
        return "Missing IPA file", 400

    ipa_file = request.files['ipa']
    token = generate_token()
    temp_dir = tempfile.mkdtemp()
    ipa_path = os.path.join(temp_dir, ipa_file.filename)
    ipa_file.save(ipa_path)

    try:
        info = extract_info_from_ipa(ipa_path)
        ipa_url = upload_to_github(ipa_path, f"{token}.ipa")

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
        plist_path = os.path.join(temp_dir, f"{token}.plist")
        with open(plist_path, "w") as f:
            f.write(plist_content)

        plist_url = upload_to_github(plist_path, f"{token}.plist")
        install_url = f"itms-services://?action=download-manifest&url={plist_url}"

        os.makedirs("static", exist_ok=True)
        qr = qrcode.make(install_url)
        qr_path = os.path.join("static", f"qr_{token}.png")
        qr.save(qr_path)

        shutil.rmtree(temp_dir)
        save_token_timestamp(token)

        return Response(f'''
        <!DOCTYPE html>
        <html>
        <head>
          <title>Install App</title>
          <style>
            body {{ background-color: #0d1117; color: white; text-align: center; font-family: sans-serif; padding: 40px; }}
            img.qr {{ margin-top: 20px; width: 200px; height: 200px; border-radius: 8px; }}
            code {{ display: block; background: #1a1a1a; color: #0f0; padding: 8px 12px; border-radius: 6px; margin-top: 15px; word-break: break-all; }}
            a.link {{ color: #61dafb; display: block; margin-top: 10px; text-decoration: none; }}
            .copy-btn {{ margin-top: 10px; background: #222; border: 1px solid #333; padding: 8px; color: white; border-radius: 4px; cursor: pointer; }}
          </style>
        </head>
        <body>
          <h2>App Installation Link</h2>
          <p>Scan the QR code or tap below to install:</p>
          <a href="{install_url}">{plist_url}</a>
          <img src="/static/qr_{token}.png" alt="QR Code" class="qr">
          <code>{install_url}</code>
          <button class="copy-btn" onclick="copyText()">Copy Link</button>
          <script>
            function copyText() {{
              navigator.clipboard.writeText("{install_url}")
                .then(() => alert("Link copied!"));
            }}
          </script>
          <a href="/" class="link">Back</a>
        </body>
        </html>
        ''', mimetype='text/html')

    except Exception as e:
        shutil.rmtree(temp_dir)
        return f"<h3 style='color:red'>Error: {str(e)}</h3>"

@app.route('/install/<token>')
def install(token):
    plist_url = f"https://raw.githubusercontent.com/{GITHUB_REPO}/{GITHUB_BRANCH}/{token}.plist"
    check = requests.get(plist_url)
    if check.status_code != 200:
        return Response(f"""
        <html><body style="background:#0d1117;color:white;text-align:center;padding:40px;font-family:sans-serif">
        <h2>Sorry, this link has expired</h2>
        <a href="/">Back to upload</a>
        </body></html>
        """, mimetype='text/html')
    return upload()

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=int(os.environ.get("PORT", 5000)))
