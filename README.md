# CN_CA_2
# Distributed File System Over LAN 

## Name: Parsa Alizadeh
## ID: 810101272

---

##  Project Goals

- Familiarize with **Distributed File Systems (DFS)**.
- Understand the use of **DFS in network environments**.
- Learn and implement **Firewall Punching** for P2P connections.
- Develop practical networking applications in **C++** using **Qt**.

---

##  Tools & Technologies

- **C++**
- **Qt Framework**
- **Reed-Solomon Error Correction Library**
- **Socket Programming**
- **Custom Encoding/Decoding and Noise Simulation**

---

##  System Overview

The system is based on two main components:

1. **Manager Node**
   - Handles meta-data storage (file size, type, chunk mapping)
   - Decides on file splitting and server assignments

2. **Chunk Servers**
   - Responsible for storing chunks of files
   - Communicate sequentially in a binary tree topology (no direct inter-server links)

A **Client Application** interacts with the Manager to perform **file write and read operations**.

---

##  How It Works

###  File Storage Process
- The client sends file details to the Manager.
- The Manager splits the file into fixed-size chunks.
- Chunks are distributed to Chunk Servers based on DFS topology.
- Each server stores its chunk and forwards the address of the next server.
- Reed-Solomon encoding is applied for error correction.
- Artificial noise is optionally added to simulate real network interference.

### File Retrieval Process
- The client requests file metadata from the Manager.
- Receives the sequence of Chunk Servers.
- Retrieves each chunk sequentially, applies Reed-Solomon decoding, and reconstructs the file.

---

##  Network Topology

A **binary tree topology** is used for Chunk Servers. The **Manager Node** and **Client** communicate directly. Chunk Servers forward files in DFS order.  


---

##  Noise Simulation

To mimic real-world data transmission conditions:
- Random noise is added to data bits using XOR.
- Reed-Solomon Error Correction is applied for recovering corrupted data.
- Error rates are controlled to ensure recoverability under threshold limits.

---

##  Firewall Punching (P2P)

For P2P communication behind firewalls:
- Implemented **UDP Firewall Punching**.
- Clients initiate outgoing requests to open NAT ports.
- Enables bi-directional data exchange without requiring static open ports.

