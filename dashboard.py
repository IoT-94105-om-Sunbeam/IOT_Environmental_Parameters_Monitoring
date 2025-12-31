from flask import Flask, render_template
import mysql.connector

app = Flask(__name__)

def get_data():
    db = mysql.connector.connect(
        host="localhost",
        user="root",
        password="root", 
        database="environment_monitoring"
    )

    cursor = db.cursor()

    cursor.execute("""
        SELECT id, temperature, humidity, gas, timestamp FROM env_data ORDER BY id DESC LIMIT 20""")

    rows = cursor.fetchall()
    db.close()
    return rows

@app.route("/")
def home():
    data = get_data()
    return render_template("dashboard.html", data=data)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
