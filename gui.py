import os
import sys
import serial
import serial.tools.list_ports
import threading

from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QPushButton, QComboBox, QLabel, QLineEdit, QTableWidget,
    QTableWidgetItem, QGroupBox, QMessageBox, QHeaderView
)

from PyQt6.QtGui import QColor, QFont, QPixmap, QIcon
from PyQt6.QtCore import Qt

# =========================
# Feature Switches (edit here)
# =========================
ENABLE_BALANCING = True   # True = show balancing controls, False = hide them

# Logo is looked up in the same folder as this script
BASE_DIR = getattr(sys, "_MEIPASS", os.path.dirname(os.path.abspath(__file__)))
LOGO_PATH = os.path.join(BASE_DIR, "cmt.png")
LOGO_HEIGHT = 60  # px, height of the logo shown at the top of the window

# =========================
# Serial Reader Thread
# =========================
class SerialReader(threading.Thread):
    def __init__(self, ser, callback):
        super().__init__(daemon=True)
        self.ser = ser
        self.callback = callback
        self.running = True

    def run(self):
        buffer = ""

        while self.running:
            try:
                if self.ser.in_waiting:
                    data = self.ser.read(self.ser.in_waiting).decode(errors='ignore')
                    buffer += data

                    while "\r\n" in buffer:
                        line, buffer = buffer.split("\r\n", 1)
                        self.callback(line.strip())

            except Exception as e:
                print("Serial error:", e)


# =========================
# Main GUI
# =========================
class BMSGui(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("BMS UART Monitor")

        if os.path.exists(LOGO_PATH):
            self.setWindowIcon(QIcon(LOGO_PATH))
        self.resize(1200, 800)

        self.ser = None
        self.reader = None

        self.ic_count = 10
        self.TEMP_COLS = 6  # number of temperature sensors shown per IC

        self.voltage_data = {i: [""] * 14 for i in range(self.ic_count)}
        self.temp_data = {i: [""] * self.TEMP_COLS for i in range(self.ic_count)}
        self.faults = [0, 0, 0, 0]

        self.MAX_PACK_VOLTAGE = 588.0  # V
        self.MAX_CHARGE_CURRENT = 10.0  # A
        self.MIN_TARGET_VOLTAGE = 2.5   # V

        self.init_ui()

    # -------------------------
    # UI Setup
    # -------------------------
    def init_ui(self):
        main = QWidget()
        layout = QVBoxLayout()

        # ===== Connection =====
        conn_box = QHBoxLayout()

        self.port_select = QComboBox()
        self.refresh_ports()

        refresh_btn = QPushButton("Refresh")
        refresh_btn.clicked.connect(self.refresh_ports)

        self.connect_btn = QPushButton("Connect")
        self.connect_btn.clicked.connect(self.toggle_connection)

        conn_box.addWidget(QLabel("COM Port:"))
        conn_box.addWidget(self.port_select)
        conn_box.addWidget(refresh_btn)
        conn_box.addWidget(self.connect_btn)

        # Logo on the right-hand side of the connection row
        logo_pixmap = QPixmap(LOGO_PATH)
        if not logo_pixmap.isNull():
            logo_label = QLabel()
            logo_label.setPixmap(
                logo_pixmap.scaledToHeight(
                    LOGO_HEIGHT, Qt.TransformationMode.SmoothTransformation
                )
            )
            conn_box.addStretch()
            conn_box.addWidget(logo_label)

        layout.addLayout(conn_box)

        # ===== Controls =====
        ctrl_box = QHBoxLayout()

        # Charging
        self.max_voltage = QLineEdit()
        self.max_voltage.setPlaceholderText("Max Voltage (V)")

        self.max_current = QLineEdit()
        self.max_current.setPlaceholderText("Max Current (A)")

        start_charge = QPushButton("Start Charging")
        stop_charge = QPushButton("Stop Charging")

        start_charge.clicked.connect(self.start_charging)
        stop_charge.clicked.connect(self.stop_charging)

        ctrl_box.addWidget(self.max_voltage)
        ctrl_box.addWidget(self.max_current)
        ctrl_box.addWidget(start_charge)
        ctrl_box.addWidget(stop_charge)

        layout.addLayout(ctrl_box)

        # Balancing (only shown when ENABLE_BALANCING is True)
        if ENABLE_BALANCING:
            bal_box = QHBoxLayout()

            self.target_voltage = QLineEdit()
            self.target_voltage.setPlaceholderText("Target Voltage (V)")

            start_bal = QPushButton("Start Balancing")
            stop_bal = QPushButton("Stop Balancing")

            start_bal.clicked.connect(self.start_balancing)
            stop_bal.clicked.connect(self.stop_balancing)

            bal_box.addWidget(self.target_voltage)
            bal_box.addWidget(start_bal)
            bal_box.addWidget(stop_bal)

            layout.addLayout(bal_box)

        # ===== Pack Statistics =====
        stats_box = QHBoxLayout()

        self.lbl_max_delta = self.make_stat_box(stats_box, "Max Cell Delta (V)")
        self.lbl_total_voltage = self.make_stat_box(stats_box, "Pack Total Voltage (V)")
        self.lbl_avg_voltage = self.make_stat_box(stats_box, "Average Cell Voltage (V)")
        self.lbl_avg_temp = self.make_stat_box(stats_box, "Average Temperature (°C)")

        layout.addLayout(stats_box)

        # ===== Tables =====
        self.voltage_table = QTableWidget(self.ic_count, 15)
        self.temp_table = QTableWidget(self.ic_count, self.TEMP_COLS + 1)

        self.voltage_table.setHorizontalHeaderLabels(
            ["IC"] + [f"C{i+1}" for i in range(14)]
        )

        self.temp_table.setHorizontalHeaderLabels(
            ["IC"] + [f"T{i+1}" for i in range(self.TEMP_COLS)]
        )

        # Spread the temperature columns evenly across the full table width
        self.temp_table.horizontalHeader().setSectionResizeMode(
            QHeaderView.ResizeMode.Stretch
        )

        layout.addWidget(QLabel("Cell Voltages (V)"))
        layout.addWidget(self.voltage_table)

        layout.addWidget(QLabel("Temperatures (°C)"))
        layout.addWidget(self.temp_table)

        # ===== Fault Status Boxes =====
        fault_box = QHBoxLayout()

        self.fault_labels = [
            self.make_stat_box(fault_box, "Undervoltage"),
            self.make_stat_box(fault_box, "Overvoltage"),
            self.make_stat_box(fault_box, "Undertemp"),
            self.make_stat_box(fault_box, "Overtemp"),
        ]
        self.lbl_fault_status = self.make_stat_box(fault_box, "Status")

        layout.addLayout(fault_box)

        main.setLayout(layout)
        self.setCentralWidget(main)

    def make_stat_box(self, parent_layout, title):
        """Creates a titled box containing a large value label and returns the label."""
        box = QGroupBox(title)
        box_layout = QVBoxLayout()

        value_label = QLabel("--")
        value_label.setAlignment(Qt.AlignmentFlag.AlignCenter)

        font = QFont()
        font.setPointSize(18)
        font.setBold(True)
        value_label.setFont(font)

        box_layout.addWidget(value_label)
        box.setLayout(box_layout)
        parent_layout.addWidget(box)

        return value_label

    # -------------------------
    # COM Ports
    # -------------------------
    def refresh_ports(self):
        self.port_select.clear()
        ports = serial.tools.list_ports.comports()
        for p in ports:
            self.port_select.addItem(p.device)

    # -------------------------
    # Connection
    # -------------------------
    def toggle_connection(self):
        if self.ser and self.ser.is_open:
            self.disconnect()
        else:
            self.connect()

    def connect(self):
        try:
            port = self.port_select.currentText()
            self.ser = serial.Serial(port, 115200, timeout=0.1)

            self.reader = SerialReader(self.ser, self.handle_line)
            self.reader.start()

            self.connect_btn.setText("Disconnect")

        except Exception as e:
            QMessageBox.critical(self, "Error", str(e))

    def disconnect(self):
        if self.reader:
            self.reader.running = False

        if self.ser:
            self.ser.close()

        self.connect_btn.setText("Connect")

    # -------------------------
    # UART Parsing
    # -------------------------
    def handle_line(self, line):
        try:
            if line.startswith("V"):
                self.parse_voltage(line)
            elif line.startswith("T"):
                self.parse_temp(line)
            elif line.startswith("F"):
                self.parse_fault(line)
        except Exception as e:
            print("Parse error:", e)

    def parse_voltage(self, line):
        # V0,4100,...
        parts = line.split(",")
        ic = int(parts[0][1:])

        values = parts[1:]

        if ic < self.ic_count:
            self.voltage_data[ic] = values[:14]
            self.update_voltage_table(ic)

    def parse_temp(self, line):
        # T0,25,26,...
        parts = line.split(",")
        ic = int(parts[0][1:])

        values = parts[1:]

        if ic < self.ic_count:
            self.temp_data[ic] = values[:self.TEMP_COLS]
            self.update_temp_table(ic)

    def parse_fault(self, line):
        # F,0,1,0,1
        parts = line.split(",")

        if len(parts) < 5:
            return

        try:
            self.faults = [int(x) for x in parts[1:5]]
            self.update_fault_table()
        except:
            pass

    # -------------------------
    # Conversions
    # -------------------------
    @staticmethod
    def raw_to_temp_c(raw):
        return raw/1000.0

    # -------------------------
    # Pack Statistics
    # -------------------------
    def update_pack_stats(self):
        # Gather every valid cell voltage (V) across all ICs
        voltages = []
        for ic in range(self.ic_count):
            for v in self.voltage_data[ic]:
                try:
                    voltages.append(float(v) / 1000.0)
                except (ValueError, TypeError):
                    pass

        # Gather every valid temperature (°C) across all ICs
        temps = []
        for ic in range(self.ic_count):
            for t in self.temp_data[ic]:
                try:
                    temps.append(self.raw_to_temp_c(float(t)))
                except (ValueError, TypeError):
                    pass

        if voltages:
            delta = max(voltages) - min(voltages)
            total = sum(voltages)
            avg_v = total / len(voltages)

            self.lbl_max_delta.setText(f"{delta:.3f}")
            self.lbl_total_voltage.setText(f"{total:.2f}")
            self.lbl_avg_voltage.setText(f"{avg_v:.3f}")
        else:
            self.lbl_max_delta.setText("--")
            self.lbl_total_voltage.setText("--")
            self.lbl_avg_voltage.setText("--")

        if temps:
            self.lbl_avg_temp.setText(f"{sum(temps) / len(temps):.2f}")
        else:
            self.lbl_avg_temp.setText("--")

    # -------------------------
    # Table Updates
    # -------------------------
    def update_voltage_table(self, ic):
        self.voltage_table.setItem(ic, 0, QTableWidgetItem(str(ic)))

        for i, v in enumerate(self.voltage_data[ic]):
            try:
                volts = float(v) / 1000.0
                item = QTableWidgetItem(f"{volts:.3f}")
                item.setBackground(self.voltage_color(volts))
                item.setForeground(QColor(0, 0, 0))
                self.voltage_table.setItem(ic, i + 1, item)
            except:
                self.voltage_table.setItem(ic, i + 1, QTableWidgetItem(""))

        self.update_pack_stats()

    def update_temp_table(self, ic):
        self.temp_table.setItem(ic, 0, QTableWidgetItem(str(ic)))

        for i, v in enumerate(self.temp_data[ic]):
            try:
                raw = float(v)

                temp_c = raw/1000.0

                item = QTableWidgetItem(f"{temp_c:.2f}")
                item.setBackground(self.temp_color(temp_c))
                item.setForeground(QColor(0, 0, 0))
                self.temp_table.setItem(ic, i + 1, item)

            except:
                self.temp_table.setItem(ic, i + 1, QTableWidgetItem(""))

        self.update_pack_stats()

    def update_fault_table(self):
        red = "background-color: rgb(255, 170, 170); color: black; border-radius: 4px; padding: 6px;"
        green = "background-color: rgb(170, 230, 190); color: black; border-radius: 4px; padding: 6px;"

        for label, val in zip(self.fault_labels, self.faults):
            label.setText("True" if val == 1 else "False")
            label.setStyleSheet(red if val == 1 else green)

        # System status box
        any_fault = any(self.faults)
        self.lbl_fault_status.setText("FAULT" if any_fault else "OK")
        self.lbl_fault_status.setStyleSheet(red if any_fault else green)

    # -------------------------
    # Command Builders
    # -------------------------
    def send(self, data: bytes):
        if self.ser and self.ser.is_open:
            self.ser.write(data)

    def start_charging(self):
        try:
            mv = float(self.max_voltage.text())   # volts
            ma = float(self.max_current.text())   # amps

            if not (0 <= mv <= self.MAX_PACK_VOLTAGE):
                QMessageBox.warning(
                    self,
                    "Invalid Voltage",
                    f"Max voltage must be between 0 and {self.MAX_PACK_VOLTAGE} V"
                )
                return

            if not (0 <= ma <= self.MAX_CHARGE_CURRENT):
                QMessageBox.warning(
                    self,
                    "Invalid Current",
                    f"Max current must be between 0 and {self.MAX_CHARGE_CURRENT} A"
                )
                return

            mv = int(mv * 10)  # correct format for elcon
            ma = int(ma * 10)  

            packet = bytes([
                0x00,
                (mv >> 8) & 0xFF,
                mv & 0xFF,
                (ma >> 8) & 0xFF,
                ma & 0xFF,
                0xFF
            ])

            self.send(packet)

        except:
            QMessageBox.warning(self, "Error", "Invalid charging inputs")

    def stop_charging(self):
        self.send(bytes([0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]))

    def start_balancing(self):
        if not ENABLE_BALANCING:
            return

        try:
            tv = float(self.target_voltage.text())   # volts

            if tv < self.MIN_TARGET_VOLTAGE:
                QMessageBox.warning(
                    self,
                    "Invalid Target Voltage",
                    f"Target voltage must be > {self.MIN_TARGET_VOLTAGE} V"
                )
                return

            if tv > self.MAX_PACK_VOLTAGE:
                QMessageBox.warning(
                    self,
                    "Invalid Target Voltage",
                    f"Target voltage must be ≤ {self.MAX_PACK_VOLTAGE} V"
                )
                return

            tv = int(tv * 10000)  # correct format for CMUs

            packet = bytes([
                0x02,
                (tv >> 8) & 0xFF,
                tv & 0xFF,
                0xFF,
                0xFF,
                0xFF
            ])

            self.send(packet)

        except:
            QMessageBox.warning(self, "Error", "Invalid target voltage")

    def stop_balancing(self):
        if not ENABLE_BALANCING:
            return

        self.send(bytes([0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]))

    # -------------------------
    # Colour Coding
    # -------------------------
    def voltage_color(self, v):
        try:
            v = float(v)
        except:
            return QColor("white")

        if 3.0 <= v <= 3.7:
            return QColor(170, 230, 190)  # green

        if 2.5 <= v < 3.0 or 3.7 < v <= 4.2:
            return QColor(255, 240, 180)  # yellow

        return QColor(255, 170, 170)  # red
    
    def temp_color(self, t):
        try:
            t = float(t)
        except:
            return QColor("white")

        if 10 <= t <= 50:
            return QColor(170, 230, 190)

        if 0 <= t < 10 or 50 < t <= 60:
            return QColor(255, 240, 180)

        return QColor(255, 170, 170)


# =========================
# Main
# =========================
if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = BMSGui()
    window.show()
    sys.exit(app.exec())