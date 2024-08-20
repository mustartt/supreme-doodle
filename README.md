# RX Compiler

### Compiler Toolchain

The compiler invocation `rxc` is the compiler driver responsible for invoking 
the different processes in the tool chain.

```
rxc: rx-fe -> opt -> llc -> link
```

### Package Format

```json
{
    "name": "package_name",
    "version": "0.0.1",
    "description": "Package Description",
    "src": ".", 
    "dependencies": [
        "io@latest",
        "http@latest",

    ]
}
```


