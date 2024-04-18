export interface LogCallback {
  onLogCallback: (logStore: string, code: number, logBytes: number, compressedBytes: number, errorMessage: string) => void;
}

export const createInstance: (endpoint: string, project: string, logstore: string, accessKeyId: string, accessKeySecret: string, accessKeyToken: string) => number;

export const destroyInstance: (token: number) => number;

export const addLog: (token: number, length: number, keyValues: string[]) => number;

export const setLogCallback: (callback: LogCallback) => void;

export const setAccessKey: (token: number, accessKeyId: string, accessKeySecret: string, accessKeyToken: string) => void;

export const setTopic: (token: number, topic: string) => void;

export const setSource: (token: number, source: string) => void;

export const addTag: (token: number, key: string, value: string) => void;

export const debuggable: (token: number, debuggable: number) => void;