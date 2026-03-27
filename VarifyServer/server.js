
const grpc = require('@grpc/grpc-js')
const message_proto = require('./proto')
const const_module = require('./const')
const {v4:uuidv4} = require('uuid')
const emailModule = require('./email')
const redis_module = require('./redis')

async function GetSecurityCode(call, callback) {
    console.log("Email is:", call.request.email)
    try{
        let redis_res = await redis_module.GetRedis(const_module.email_prefix+call.request.email);
        let uniqueId = "";
        if (redis_res == null){
            uniqueId = uuidv4().toUpperCase().substring(0,4);
            let bres = await redis_module.SetRedisExpire(const_module.email_prefix+call.request.email, uniqueId, 600);
            if(!bres){
                callback(null, { email:  call.request.email,
                    error:const_module.Errors.RedisError
                });
                return;
            }
        }
        else{
            uniqueId = redis_res;
        }
        console.log("UniqueId is ", uniqueId)
                let html_str = `
                <!DOCTYPE html>
                <html>
                <head>
                    <meta charset="utf-8">
                    <style>
                        .container { max-width: 600px; margin: 0 auto; font-family: Arial, sans-serif; }
                        .header { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 20px; text-align: center; }
                        .content { padding: 30px; background: #f8f9fa; }
                        .code-box { background: white; padding: 25px; border-radius: 10px; text-align: center; margin: 20px 0; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
                        .verification-code { font-size: 42px; font-weight: bold; color: #e74c3c; letter-spacing: 8px; margin: 15px 0; }
                        .warning { color: #e74c3c; font-weight: bold; background: #ffeaa7; padding: 10px; border-radius: 5px; margin: 15px 0; }
                        .footer { text-align: center; color: #666; font-size: 12px; padding: 20px; }
                    </style>
                </head>
                <body>
                    <div class="container">
                        <div class="header">
                            <h1>QuickChat 账户注册验证</h1>
                        </div>
                        <div class="content">
                            <p>亲爱的用户，您好！</p>
                            <p>您正在注册QuickChat账户，请使用以下验证码完成验证：</p>
                            
                            <div class="code-box">
                                <p>验证码</p>
                                <div class="verification-code">${uniqueId}</div>
                                <p style="color: #666;">(有效期3分钟)</p>
                            </div>
                            
                            <div class="warning">
                                ⚠️ 安全提示：请勿向任何人泄露此验证码！
                            </div>
                            
                            <p>如果这不是您的操作，请忽略此邮件。</p>
                        </div>
                        <div class="footer">
                            <p>系统自动发送，请勿回复</p>
                        </div>
                    </div>
                </body>
                </html>
                        `;
        //发送邮件
        let mailOptions = {
            from: 'v125250@163.com',
            to: call.request.email,
            subject: '验证码',
            html: html_str,
        };

        let send_res = await emailModule.SendMail(mailOptions);
        console.log("Send Result Is ", send_res)

        if (!send_res){
            callback(null, { email:  call.request.email,
                error:const_module.Errors.Success
            }); 
        }


    }catch(error){
        console.log("Catch Error Is ", error)

        callback(null, { email:  call.request.email,
            error:const_module.Errors.Exception
        }); 
    }
 
}

function main() {
    var server = new grpc.Server()
    server.addService(message_proto.VarifyService.service, { GetSecurityCode: GetSecurityCode })
    server.bindAsync('0.0.0.0:50051', grpc.ServerCredentials.createInsecure(), () => {
        console.log('grpc server started')        
    })
}

main()