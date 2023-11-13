# react-native-msg-pack

just msg pack wrapper for rn

## Installation

```sh
npm install react-native-msg-pack
```

## Usage

```js
import { parse, encode, install } from 'react-native-msg-pack';

// ...
install();
const encoded = encode({a: 10});
const result = parse(encoded);
assert(result.a == 10);
```

## Contributing

See the [contributing guide](CONTRIBUTING.md) to learn how to contribute to the repository and the development workflow.

## License

MIT

---

Made with [create-react-native-library](https://github.com/callstack/react-native-builder-bob)
