import hilog from '@ohos.hilog';

import { AliyunLog, LogCallback } from "@aliyunsls/producer"

let aliyunLog: AliyunLog = new AliyunLog(
  "<your endpoint>",
  "<your project name>",
  "<your logstore name>",
  "<your AccessKey id>",
  "<your AccessKey secret>",
  "<your AccessKey token>" // 只有当AccessKey是通过STS方式获取时才需要
);

// （可选）设置Topic
aliyunLog.setTopic("test");
// （可选）设置Source，默认为HarmonyOS，不建议修改
aliyunLog.setSource("test-source");
// （可选）设置Tag
aliyunLog.addTag("tst", "value");
// （可选）开启debug模式，建议仅在遇到问题时开启
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
      "<your accesskey id>", // 填入正确的AccessKey Id
      "<your accesskey secret>", // 填入正确的AccessKey Secret
      null // 填入正确的AccessKey Token. 只有当AccessKey是通过STS方式获取时才需要
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