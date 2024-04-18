import {
  createInstance,
  destroyInstance,
  addLog as add,
  setLogCallback as setCallback,
  setAccessKey as setAK,
  setTopic,
  setSource,
  addTag,
  debuggable,
  LogCallback
} from "libaliyunlog.so"

class AliyunLog {
  private token: number;

  constructor(endpoint: string, project: string, logstore: string, accessKeyId: string, accessKeySecret: string, accessKeyToken: string = null) {
    this.token = createInstance(endpoint, project, logstore, accessKeyId, accessKeySecret, accessKeyToken);
  }

  /**
   * Add Log to AliyunLog SDK.
   * @param log Map<string, string>
   * @returns
   */
  addLog(log: Map<string, string>): number {
    if (null == log) {
      return -1;
    }

    let len = log.size;
    let keyValues: string[] = new Array(len * 2);
    let index = 0;

    for (let [key, value] of log.entries()) {
      keyValues[index * 2] = key;
      keyValues[index * 2 + 1] = value;
      index++;
    }

    return add(this.token, len, keyValues);
  }

  setLogCallback(callback: LogCallback): void {
    setCallback(callback);
  }

  setAccessKey(id: string, secret: string, token: string = null) {
    setAK(this.token, id, secret, token);
  }

  setTopic(topic: string) {
    setTopic(this.token, topic);
  }

  setSource(source: string) {
    setSource(this.token, source);
  }

  addTag(key: string, value: string) {
    addTag(this.token, key, value);
  }

  debuggable(debug: boolean) {
    debuggable(this.token, debug ? 1 : 0);
  }

  destroy() {
    destroyInstance(this.token);
  }
}

export { AliyunLog }
export { LogCallback } from "libaliyunlog.so"