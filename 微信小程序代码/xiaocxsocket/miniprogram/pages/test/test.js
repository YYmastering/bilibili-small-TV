const app = getApp()
//let Utf8ArrayToStr = require('../../utils/Utf8ArrayToStr.js');
var ip;
var port;//本地端口
var ipzi;
Page({
  data: {
    sign:0,//标志位，用于前端判定
    ipzi2:""//读取到前端的ip
  },

  // send: function(e) {

  //   // 向指定的 IP 和 port 发送消息
  //   this.udp.send({
  //     address: '192.168.199.149',
  //     port: '1234',
  //     message: 'i want you'
  //   })
  // },

chatime(){
  this.udp.send({
    address: ip,
    port: '1234',
    message: "time"
  })
},
chawen(){
  this.udp.send({
    address: ip,
    port: '1234',
    message: 'wen'
  })
},
chafore(){
  this.udp.send({
    address: ip,
    port: '1234',
    message: 'fore'
  })
},
chaliu(){
  this.udp.send({
    address: ip,
    port: '1234',
    message: 'liu'
  })
},
chabili(){
  this.udp.send({
    address: ip,
    port: '1234',
    message: port.toString()
  })
},

//向esp8266发送消息，查询是否在线
udplink(res){
  wx.showLoading({
    title: '正在连接',
    mask:true
  })
  console.log(res.detail.value.ip);
  if(res.detail.value.ip!=0){
  ip=res.detail.value.ip;
  this.udp.send({
    address: ip,
    port: '1234',
    message: "ready"
  })
 setTimeout(res=>{  this.udp.send({
  address: ip,
  port: '1234',
  message: ipzi
})},200)
setTimeout(res=>{  this.udp.send({
  address: ip,
  port: '1234',
  message: port.toString()
})},200)
setTimeout(res=>{ 
if(this.data.sign==1)
{ wx.showToast({
  title: '连接成功！',
})}
else{wx.showToast({
  title: '连接失败',
  icon:"error"
})}
wx.hideLoading()
},1500)


  }else{
    wx.showToast({
    title: '请填写IP地址',
    icon:"error"
  })
wx.hideLoading()
}
},

//arraybuffer转string
Utf8ArrayToStr(array) {
  var out, i, len, c;
  var char2, char3;

  out = "";
  len = array.length;
  i = 0;
  while (i < len) {
    c = array[i++];
    switch (c >> 4) {
      case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
        out += String.fromCharCode(c);
        break;
      case 12: case 13:
        char2 = array[i++];
        out += String.fromCharCode(((c & 0x1F) << 6) | (char2 & 0x3F));
        break;
      case 14:
        char2 = array[i++];
        char3 = array[i++];
        out += String.fromCharCode(((c & 0x0F) << 12) |
          ((char2 & 0x3F) << 6) |
          ((char3 & 0x3F) << 0));
        break;
    }
  }
  return out;
},


  // 页面加载完成事件由系统调用
  onLoad: function () {
    var that=this
    // 新建udp实例
    this.udp = wx.createUDPSocket()//创建套接字
    port= this.udp.bind()//绑定udp，返回值为本地随机监听的udp端口
    console.log("本地端口为"+port)//打印本地监听的端口
  //监听udp收到的信息
    
  this.udp.onMessage((res)=>{
        console.log(res.message)//收到的信息为arraybuffer类
        let messageStr = this.Utf8ArrayToStr(new Uint8Array(res.message))//转换为string
        console.log(messageStr)//打印最终信息
        if(messageStr=="linked")
        {that.setData({sign:1})
     
      }
        }),

//获取本机内网ip
    wx.getLocalIPAddress({
      success (res) {
        console.log(res.localip)//打印本机内网IP
         ipzi=res.localip
        that.setData({ipzi2:ipzi})//修改ipzi2，发送到esp8266端
    
      }
    })
  },
  //留言变更事件
  xieliuyan(res){
    console.log(res.detail.value)
    var lyb=res.detail.value
    //先发送“liuyan”告诉esp8266接下来来的信息是留言板上面的
    this.udp.send({
      address: ip,
      port: '1234',
      message: "liuyan"
    })
    //延迟200ms发送留言信息
    setTimeout(res=>{  this.udp.send({
      address: ip,
      port: '1234',
      message: lyb
    })},200)
  }
})
