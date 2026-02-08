import requests

BASE_URL = "http://localhost:8080"  

def test_hello():
    r = requests.get(f"{BASE_URL}/hello")
    assert r.status_code == 200
    assert r.json()["message"] == "Hello World"

def test_page():
    r = requests.get(f"{BASE_URL}/page")
    assert r.status_code == 200
    assert "<h1>Welcome!" in r.text

def test_data():
    r = requests.get(f"{BASE_URL}/data")
    assert r.status_code == 200
    assert "<value>42</value>" in r.text

def test_namaskar():
    r = requests.get(f"{BASE_URL}/namaskar")
    assert r.status_code == 200
    assert "Namaskar!" in r.text

def test_json():
    r = requests.get(f"{BASE_URL}/json")
    assert r.status_code == 200
    assert r.json()["message"] == "Hello JSON"
''''
def test_submit():
    payload = "TestData"
    r = requests.post(f"{BASE_URL}/submit", data=payload)
    assert r.status_code == 200
    assert f"Data received: {payload}" in r.text
'''
def test_submit():
    payload = {"data": "TestData"}
    r = requests.post(f"{BASE_URL}/submit", json=payload)
    assert r.status_code == 200
    assert "TestData" in r.text
