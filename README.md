**The Adaptive Radix Tree:**

Implement the Adaptive Radix Tree, therefore define the following methods:

- lookup
- insert

## Structure of Node types 4, 16, 48s and 256.

- Node 4: sorted keys, 4 keys 4 pointers (2 arrays)
- Node 16: 16 Keys, 16 pointers
- Node 48: 256 child indices, 48 child pointers
- Node 256: 256 child pointers

The Key type already allows to address it bytewise (see 1 test case).

## My Scripts

```bash
bash run.sh
```

```bash
bash profiling.sh
```

Build and run:
`mkdir build`
`cd build`
`cmake ..`
`make`
`./art_test`
