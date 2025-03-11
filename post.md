### Building Proof of Presence: A First Microcontroller Project

The political challenges we face today are particularly pronounced in large-scale systems, which often lack the agility and accountability needed to address complex issues effectively. In contrast, democratic processes tend to thrive at the local level, where stronger community engagement and more adaptable governance structures allow for quicker responses to specific needs. This disparity underscores the importance of shifting focus toward decentralized and localized systems. By empowering communities to maintain autonomy and self-determination, such systems can also serve as a critical counterbalance to the rise of authoritarian tendencies that often emerge in centralized, macro-level governance.

One promising avenue for supporting these decentralized systems lies in the development of technologies like proof of presence (PoP) and proof of location (PoL). PoP verifies that an entity exists at a specific moment in time, creating a foundation for PoL, which confirms the entity’s precise location. This logical progression—from establishing presence to verifying location—provides a clear framework for building trustworthy, localized systems that can operate with integrity and transparency. 

While existing projects exploring decentralized location protocols have shown some potential, they often struggle to deliver practical, scalable solutions. To address these limitations, this project focuses on developing a microcontroller-based open source system to advance PoP technology. The aim is to contribute to the creation of decentralized systems that not only strengthen local governance and community-driven initiatives but also bolster resilience against the centralization of power and the threats posed by authoritarianism.

---

### The Idea and its Problems

At the heart of this project is the idea of creating a unique device that could prove both location and presence. My goal was to design something that could generate time-based one-time passwords (TOTP) and display them as scannable QR codes that could then be verified for authenticity. During my research, I came across a company called [Skiply](https://www.skiply.eu/en/ubiqod-key-2/), which develops devices that display time-based QR codes for verification purposes. Their work caught my interest and inspired me to create an open-source alternative - a device that could achieve similar functionality, but could be built by anyone.

However, the challenge goes beyond simply generating valid TOTP-based QR codes. The real difficulty lies in ensuring that these codes act as true proof of presence. Because TOTP codes are only valid for a 30-second window, anyone attempting to verify the QR code must be physically close to the device to scan it in real time. This creates a strong link between the code and the physical location of the device.

But even with this time constraint, there's still a vulnerability: relay attacks. In such attacks, an attacker could capture a valid QR code - whether through a photo, live video or shared link - and forward it to someone else for verification. This would allow the attacker to fake their presence and undermine the integrity of the system. To address this issue, one possible solution is to implement a server-side cooldown period between successful validations. By restricting how frequently a device can generate valid proof of presence, a potential attacker attempting to relay a scan would be forced to wait before making another attempt.

---

### The Hardware Setup

![Device Setup](pictures/device.jpeg?raw=true)

The first challenge was to ensure the device could keep accurate time without relying on an internet connection. I started with an ESP8266 microcontroller and its built-in Wi-Fi module, but I wanted the device to work completely offline. A friend lent me a GPS module, and I discovered that it could also receive accurate time data from GPS satellites.

The hardware setup:

- **ESP32 Microcontroller**: Upgrade from the ESP8266 because I needed more GPIO pins.
- **NEO-6M GPS Module**: To acquire location and time data.
- **DS3231 Real-Time Clock (RTC) Module**: To maintain accurate time.
- **Waveshare 1.54" Black/White E-Paper Display**: I chose a black-and-white display over a color one because the refresh rate is much faster (2 seconds vs. 8 seconds).
- **2N2222 NPN Bipolar Transistor**: To switch off the GPS module once it acquires the location and time.
- **Breadboard and Cables**: For prototyping.
- **5.1K Resistor**: For circuit stability.

---

### Operation breakdown

1. **Boot-Up**: On startup, the device begins acquiring a GPS signal to get the current time and location. This process usually takes up to 2 minutes.
2. **GPS Shutdown**: Once the GPS module has acquired GPS data, it’s turned off using the 2N2222 transistor.
3. **TOTP Generation**: The device waits until the next half-minute mark, then generates a TOTP number using a hard-coded key. 
4. **QR Code Creation**: A link is generated containing the device ID and the encrypted TOTP number. In the future, I want to add the location data as well (which also would have to be encoded).
5. **Display**: The QR code is displayed on the e-paper screen. To make it easier for users to verify the device, I added a small square in the center of the screen showing the first two and last two digits of the TOTP number.
6. **Deep Sleep**: After rendering the QR code, the device goes into deep sleep to save power (27 seconds).

![QR Code Display](pictures/screen.jpeg?raw=true)

I set up a Flask server to verify the QR codes. When a code is scanned, the server returns a JSON response confirming the validity.

At this stage, the device is fully functional. It can generate and display QR codes, and the Flask server can verify them.

![Verification JSON](pictures/verification.jpeg)

---

### Next Steps

1. **Hardware Security Module (HSM)**: I’ve ordered an HSM module to handle TOTP creation and encryption, which will prevent device duplicates.
2. **Web Implementation**: More user-friendly web interface for device registration and verification.
4. **Battery Module**: Adding a battery will make the device portable.
5. **Map Integration**: A map to visualize registered devices and their locations.
6. **Casing Design**: Designing a 3D-printed casing

If you’re interested in following this project, feel free to reach out or leave a comment. I’ll be sharing updates of my progress!

[Project on GitHub](https://github.com/sweing/TOTP-QR-Gen?raw=true)


---