import network
import time
import urequests
from machine import UART

# ---------- WiFi ----------
SSID = "  "
PASSWORD = "  "

sta = network.WLAN(network.STA_IF)
sta.active(True)
sta.connect(SSID, PASSWORD)
while not sta.isconnected():
    pass
print("✅ WiFi Connected:", sta.ifconfig())

# ---------- UART 初始化 ----------
uart = UART(2, baudrate=9600, tx=17, rx=16)

# ---------- 城市清單 ----------
cities = ["Taipei", "Tokyo", "Seoul", "Beijing", "Hanoi", "Paris", "London", "Berlin"]
API_KEY = "46a3cc4701eae664d989aa08ec842c56"

def get_weather(city):
    url = f"https://api.openweathermap.org/data/2.5/weather?q={city}&units=metric&appid={API_KEY}"
    try:
        r = urequests.get(url)
        data = r.json()
        r.close()
        temp = round(data["main"]["temp"])
        t = time.localtime(data["dt"] + data["timezone"])
        hour = t[3]
        minute = t[4]
        return f"{hour:02d}:{minute:02d} {temp:02d}\n"
    except:
        return None

# ---------- 每分鐘重傳所有城市資料 ----------
while True:
    print("🚀 Fetching and sending all city data...")
    for city in cities:
        msg = get_weather(city)
        if msg:
            uart.write(msg)
            print("📤", city, "→", msg.strip())
        else:
            uart.write("------ --\n")
            print("⚠️ Failed:", city)
        time.sleep(0.5)  # 每筆間隔 0.5 秒
    uart.write("DONE\n")
    print("✅ All data sent. Waiting 60 seconds...")
    time.sleep(60)
