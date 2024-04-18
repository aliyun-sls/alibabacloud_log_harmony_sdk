import hilog from '@ohos.hilog';

// import { AliyunLog, LogCallback } from "sharedlibrary"
import { AliyunLog, LogCallback } from "@aliyunsls/producer"

let aliyunLog: AliyunLog = new AliyunLog(
  "https://cn-beijing.log.aliyuncs.com",
  "<your project name>",
  "<your logstore name>",
  "LTe*********",
  "DYX**********",
  "" // 仅当AccessKey是通过STS方式获取时才需要
);

aliyunLog.setTopic("test");
aliyunLog.setSource("test-source");
aliyunLog.addTag("tst", "value");
aliyunLog.debuggable(false);

aliyunLog.addLog(new Map(
  [
    ["key1", "value1"],
    ["key2", "value2"],
    ["key3", "value3"],
  ]
));

class MyLog implements LogCallback {

  init() {
    aliyunLog.setLogCallback(this);
  }

  addLog() {
    aliyunLog.addLog(new Map([
      ["key1", "val1"],
      ["key2", "val2"],
      ["key3", "val3"],
      ["key4", "val4"],
    ]))
  }

  onLogCallback(logStore: string, code: number, logBytes: number, compressedBytes: number, errorMessage: string) {
    hilog.info(0x0000,
      'testTag',
      'onLogCallback. logStore: %{public}s, code: %{public}d, logBytes: %{public}d, compressedBytes: %{public}d, errorMessage: %{public}s',
      logStore, code, logBytes, compressedBytes, errorMessage
    );
    aliyunLog.setAccessKey(
      "LTe********", // 填入正确的AccessKey Id
      "DYX**********", // 填入正确的AccessKey Secret
    );
  }
}

@Entry
@Component
struct Index {
  @State message: string = 'Hello World'
  private log: MyLog = new MyLog();

  build() {
    Row() {
      Column() {
        Text("Init")
          .fontSize(50)
          .fontWeight(FontWeight.Bold)
          .onClick(() => {
            this.log.init();
          });
        Text("Add")
          .fontSize(50)
          .fontWeight(FontWeight.Bold)
          .onClick(() => {
            this.log.addLog();
          })
      }
      .width('100%')
    }
    .height('100%')
  }
}