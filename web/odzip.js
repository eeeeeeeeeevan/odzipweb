export default async function Odzip() {
    const Module = await OdzipModule();

    const _compress = Module.cwrap('odz_wasm_compress', 'number', ['number', 'number']);
    const _decompress = Module.cwrap('odz_wasm_decompress', 'number', ['number', 'number']);
    const _strerror = Module.cwrap('odz_wasm_strerror', 'string', ['number']);
    const _free = Module.cwrap('odz_wasm_free', null, ['number']);

    function call(fn, input) {
        const ptr = Module._malloc(input.length);
        Module.HEAPU8.set(input, ptr);

        const resPtr = fn(ptr, input.length);
        Module._free(ptr);

        const err = Module.getValue(resPtr, 'i32');
        const data = Module.getValue(resPtr + 4, 'i32');
        const size = Module.getValue(resPtr + 8, 'i32');

        if (err !== 0) {
            throw new Error(_strerror(err));
        }

        const out = new Uint8Array(size);
        out.set(Module.HEAPU8.subarray(data, data + size));
        _free(data);
        return out;
    }

    return {
        compress(input) {
            if (!(input instanceof Uint8Array)) {
                throw new TypeError('uint8arr');
            }
            return call(_compress, input);
        },
        decompress(input) {
            if (!(input instanceof Uint8Array)) {
                throw new TypeError('uint8arr');
            }
            return call(_decompress, input);
        },
        strerror: _strerror,
    };
}