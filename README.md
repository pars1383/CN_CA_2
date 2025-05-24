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
- Implemented **UDP Firewall Punching**.
- Clients initiate outgoing requests to open NAT ports.
- Enables bi-directional data exchange without requiring static open ports.







## How are noise and transmission errors handled in networks?

During data transmission, errors and noise are managed using error detection and correction algorithms. In this project, we utilized Reed-Solomon Error Correction which adds parity bits to the data. Upon receiving data, the receiver can detect and correct up to t/2 bits of error within a chunk. If the error rate exceeds this limit, the chunk is flagged as corrupt. This method ensures a higher level of data integrity over unreliable or noisy networks.

For P2P communication behind firewalls:
- Implemented **UDP Firewall Punching**.
- Clients initiate outgoing requests to open NAT ports.
- Enables bi-directional data exchange without requiring static open ports.

## Why do video streaming platforms use UDP, and how do they handle errors?

Video streaming platforms like YouTube, Twitch, and Netflix prefer UDP because it provides low-latency, connectionless data transfer without the overhead of retransmissions like TCP. Since video streams are continuous and real-time, missing a few packets is less critical than delaying the stream. These platforms handle errors by:

    Using buffering to absorb minor losses.

    Applying forward error correction (FEC) techniques.

    Skipping corrupt frames or substituting with predicted frames.
    This trade-off ensures smoother playback at the expense of occasional quality drops.

## How are storage-time errors handled, and what advantages do distributed file systems offer here?

When data is stored (at rest), it can still become corrupt due to hardware failures, bit rot, or accidental overwrites. Distributed File Systems (DFS) mitigate this risk by:

    Storing multiple replicas of each file across different nodes.

    Monitoring data health through periodic integrity checks.

    Replacing corrupted replicas with healthy ones from other servers.
    This redundancy ensures higher availability and data resilience compared to centralized systems.
## Based on the noise and denoise parameters you used, what percentage of transmitted data was unrecoverable or undetectable?

In our simulations, by controlling the noise probability and error correction capacity (fec_length and t value in Reed-Solomon), we observed that:

    Approximately 5-10% of data chunks experienced bit errors during transmission.

    Thanks to error correction, less than 1-2% of those errors were unrecoverable.

    Undetectable errors (errors that bypassed correction without detection) were negligible due to the parity overhead designed to cover the expected noise rate.

## Do distributed file systems prevent all data failures? What is the role of replicas in these systems?

Distributed file systems cannot prevent all possible failures but significantly reduce their impact through:

    Replication: Storing multiple copies of each data chunk across different nodes.

    Failover handling: If one node or chunk is lost or corrupted, other replicas can serve the data.

    Load balancing: Distributing read/write operations across replicas to avoid overloading a single point.
    Replicas increase system fault tolerance, availability, and data durability in case of hardware or network issues.
## How can distributed file systems help eliminate bottlenecks?

In traditional centralized systems, data requests funnel through a single server, creating bottlenecks. Distributed File Systems:

    Distribute data across multiple servers/nodes.

    Allow parallel access to different data chunks.

    Implement hierarchical or balanced network topologies (like binary trees or mesh networks) to distribute traffic evenly.
    This architecture eliminates single points of congestion, increasing scalability and throughput.

## distributed file systems. How do both reduce access time?

CDNs and DFS are similar in that both aim to bring data closer to the end-user, but they serve different purposes:

    CDNs cache and serve static content (videos, images, scripts) geographically near users to reduce latency.

    DFS focuses on distributed storage and retrieval with redundancy, fault tolerance, and concurrent access.

Both reduce access times by:

    Minimizing the physical distance between data and user.

    Balancing load across multiple nodes/servers.

    Replicating data strategically.

