# Raspberry Pi 5 GPU GHZ Scaling Evidence

Status: PASS

This evidence records a standalone QBIT NOVA GPU compute bridge prototype.

Boundary:
- Software virtual QCPU on classical Raspberry Pi 5 hardware.
- Not physical quantum hardware.
- Not wired into QBIT NOVA Language grammar yet.
- QBIT NOVA C Devpost project was not touched.
- Pi Pico was not flashed or modified.

Verified host:
- Host: nova-pi
- Architecture: aarch64
- Kernel: 6.18.34+rpt-rpi-2712
- GPU renderer: V3D 7.1.7.0
- API: OpenGL ES 3.1
- Mesa: 25.0.7-2+rpt4

Verified workload:
- GHZ-16: GPU matched CPU reference.
- GHZ-18: GPU matched CPU reference.
- GHZ-20: GPU matched CPU reference.
- GHZ-22: GPU matched CPU reference.

Scaling summary:
- 16 qubits: GPU slower due to launch overhead.
- 18 qubits: GPU faster.
- 20 qubits: GPU faster.
- 22 qubits: GPU about 5x faster than CPU in this test.

Accuracy:
- Maximum amplitude error: 0.00000001
- Result: GPU_MATCHES_CPU_REFERENCE

Thermal:
- Before: 53.8 C
- After: 54.3 C
- Throttling: 0x0

Evidence log:
- /home/aslam/qbit-gpu-bridge/gpu_scaling_20260727T051712Z.log
- SHA256: 271ff48fa2a29b12fa473fa5d58e804dd8d20f8c9101a4616da246ff4c3ff2d1

Truth boundary:
This proves GPU compute-shader acceleration for the tested GHZ state-vector workload.
It does not prove physical quantum computation, AI model inference, or production language integration.
