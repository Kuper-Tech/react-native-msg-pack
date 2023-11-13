import * as React from 'react';
import { StyleSheet, View, Text } from 'react-native';
import { parse, encode, install } from 'react-native-msg-pack';
import { serialize as encodeJs, deserialize as parseJs } from '@ygoe/msgpack';
const largeJson = require('./order.json');

install();
const encodedArrayBuffer = encode(largeJson);
const encodedString = encodeJs(largeJson);
const encodedPlainJson = JSON.stringify(largeJson);

let before = 0;
let after = 0;
let someJSON: unknown;
export default function App() {
  let jsiValue = 0;
  let pureJsValue = 0;
  let jsonValue = 0;
  React.useEffect(() => {
    for (let i = 0; i < 100; i++) {
      before = performance.now();
      someJSON = parse(encodedArrayBuffer);
      after = performance.now();
      jsiValue += (after - before);
    }
    console.log('json', someJSON);
    console.log(`JSI parsing time: ${jsiValue / 100} ms`);

    for (let i = 0; i < 100; i++) {
      before = performance.now();
      someJSON = parseJs(encodedString);
      after = performance.now();

      pureJsValue += (after - before);
    }
    console.log('json', someJSON);
    console.log(`pure js parsing time: ${pureJsValue / 100} ms`);

    for (let i = 0; i < 100; i++) {
      before = performance.now();
      someJSON = JSON.parse(encodedPlainJson);
      after = performance.now();
      jsonValue += (after - before);
    }

    console.log('json', someJSON);
    console.log(`JSON.parse parsing time: ${jsonValue / 100} ms`);

  }, []);

  return (
    <View style={styles.container}>
      <Text>Nothing</Text>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
  },
  box: {
    width: 60,
    height: 60,
    marginVertical: 20,
  },
});
