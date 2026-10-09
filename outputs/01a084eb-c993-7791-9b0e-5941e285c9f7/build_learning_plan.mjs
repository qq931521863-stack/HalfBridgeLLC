import fs from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { Workbook, SpreadsheetFile } from '@oai/artifact-tool';

const outDir = path.dirname(fileURLToPath(import.meta.url));
const project = 'D:/Work/1600W/代码分析';
const study = 'D:/study/power/buck/失效模型';
// task: question, knowledge, steps, evidence/acceptance, variation, minutes, level, source.
const stages = [
 {id:'S01',name:'项目共识与基线',gate:'能说清系统边界、模型负载和已知/未知规格；可复跑同一工作点。',tasks:[
 ['确认资料与版本','代码、DLL、模型和原理图是不同版本对象','列出正在用的四个文件/版本；选一个学习副本；不比较全部历史文件。','一张版本对应表；不确定的匹配明确标未知。','换一个模型文件，能指出哪个接口/参数需重核。',15,'A','R01；R03'],
 ['冻结已知规格与未知项','AC输入、DC母线、电池范围、功率与电流的区别','填项目规格表；沿用400V、380～420V、30～60V事实；列最大电流/CV目标等未知项。','已知值有来源；未知项没有猜填；算1600W在30V和60V的电流要求。','为何60V上限不能直接当所有电池的CV目标？',25,'B','R01'],
 ['画整机四条路径','能量流、采样链、控制链、故障链','对照原理图圈PFC/LLC/SR/输出；各画一条路径；只标实际能确认的节点。','一张标注图，能指出软件作用位置及继电器前后节点。','继电器断开且Ibat≈0，原边是否一定无电流？',25,'A','R01；R04 §1'],
 ['保存开环基线','负载、初态、器件理想化和观察窗口','在已能运行的模型原参数下复跑；记录Vbus/fs/调制/负载；保存门极与输出波形。','同版本能复跑；写清电阻、电池或其他负载及未建因素。','仍连接的理想0V电压源等于无电池吗？',25,'B','R03 §11；R05 L5'],
 ['统一量与符号','瞬时/平均/RMS；节点参考；ir/im/ip/Ibat','给Vbus、SW、Vcr、ir、im、Ibat写单位和方向；在图上标参考点。','一页符号表；能说明ir=im+ip不是输出直流电流关系。','SW的0/400V与去直流后的±200V是否矛盾？',25,'B','R04 §1.4–1.7'],
 ['建立记录与学习共识','一个问题、预测、证据、复述、变式','用实验记录填写上一项结果；练一次限定AI提问；约定只核对当前问题。','一张自己的记录及一个25分钟下一步；区分A/B与实机证据。','AI写出答案且你读懂了，为什么还不能标通过？',15,'A','R03 §4、§9；R05 §2、§8']
 ]},
 {id:'S02',name:'必要电路与数学基础',gate:'带单位独立计算斜率、能量、周期和变比；说清初值与适用条件。',tasks:[
 ['把单位和周期算对','μ/n/k；Hz、s、计数单位','算125kHz周期；把20μH、164nF换SI单位；检查一条代码量纲。','三个带单位的计算；能识别Hz与tick的不同。','频率加倍，周期怎样变化？控制更新频率也一定加倍吗？',25,'B','R04 §2、§4'],
 ['理解电感电流斜率','v=L·di/dt与初始电流','选一个固定电压算例，先算Δi再加i0；反向电压重算；对应Lr或Lm。','能算增量和终值；知道关门极不清除电感能量。','同一脉冲但i0不同，峰值能相同吗？',25,'B','R04 §2.1'],
 ['理解电容电压斜率','i=C·dv/dt与输出电流差','用给定Co、整流电流和负载电流算dVo/dt；改成电池负载说明差异。','能写Co·dVo/dt=irect-iload并说明各电流。','有Vo但LLC没传能，可能是什么原因？',25,'B','R04 §2.2；R01 §1.3'],
 ['理解变压器与伏秒','匝比、极性、磁通累积','从实际副边接法定义Np/Ns；算一组电压/电流折算；解释不对称伏秒。','极性和Ns定义明确；模型值没有冒充实物设计。','中心抽头半绕组与全绕组的Ns能混用吗？',50,'B','R04 §2.3、§4.3'],
 ['核对储能与功率','LI²/2、CV²/2、输入输出损耗','算一个Lr和Co能量；比较输入、输出测量点；区分稳态平均与瞬时功率。','带初值和单位的能量表；解释损耗和能量暂存。','为什么瞬时输入输出功率可能不相等？',25,'B','R04 §2；R01 §2'],
 ['理解RMS与基础复数','RMS损耗；串并联阻抗；相位','算简单矩形电流RMS；按讲义写ZL/ZC及一个分压式；先不推全套模型。','能区分平均电流与RMS；能在fr检查串联电抗抵消。','Zs=0是否等于整个输入阻抗为0？',50,'B','R04 §5']
 ]},
 {id:'S03',name:'半桥驱动与原边时序',gate:'能从模式、频率、Duty和许可推导门极窗口，并核对代码/模型差异。',tasks:[
 ['识别半桥四种开关状态','QH/QL、SW、死区内续流','填高开低关/高关低开/全关/同时开表；把实际驱动极性标出来。','表中全关状态没有被写成固定零电压；能解释直通路径。','两管全关时，SW为什么可能继续变化？',25,'A','R04 §1.2；R02 §2'],
 ['计算短脉冲PWM窗口','T/4与3T/4中心；Duty到边沿','按SHRTIMERdrive非模式7分支，独立算125kHz、Duty=0.2低侧脉宽；再算四边沿。','手算带单位，能对照源码；尚未提交的前一题在这里完成。','改频率或Duty，是否会变成20%/80%互补？',25,'B','R03 §8；R08 SHRTIMERdrive'],
 ['计算PFM与窗口间隔','频率到周期；固定计数偏移','按模式7算周期/四边沿/相邻窗口间隔；区分120计数偏移与240计数间隔。','命令窗口间隔计算正确；不把计数直接读作ns。','同模式7改变Duty，原边窗口是否一定变化？',25,'B','R08 SHRTIMERdrive；R02 §7.3'],
 ['核对模型门极生成方式','锯齿、边沿、使能和归一化','在原模型观察一周期；追原边门极连接；对照短脉冲或互补公式。','记录指令与实际逻辑门极；不同调制明确标差别。','Duty非0但DrvH=0，上管应该有什么波形？',50,'B','R09 DLL出口；R03 T01'],
 ['建立驱动供电认识','VDD、VB−VS、UVLO与传播延迟','从原理图找原边驱动型号；区分自举和隔离供电；列缺失参数与测量需求。','一张供电/信号路径表；没有型号时保留未知，不给固定预充时间。','先充过自举，停波0.5s后还能直接沿用吗？',50,'A','R02 §3'],
 ['核对MCU时序初始条件','预装载、生效点、GPIO复用和故障封锁','读HRTIM初始化与门极使能；标安全帧生效和许可顺序；先做纸面时序。','一张第一拍事件表；已配置和已实测分别标注。','比较值写好了是否等于已锁存且引脚已安全输出？',50,'A','R10 HRTIM；R02 §7.5']
 ]},
 {id:'S04',name:'LLC功率对象与开环实验',gate:'能解释一个周期的传能和两个邻近工作点；知道静态模型与ZVS的边界。',tasks:[
 ['定位Lr、Cr、Lm','实体器件、漏感和励磁等效','对照实际图和模型标三参数位置及来源；核对是否与FullTest参数相同。','参数表含来源/未知；不把模型Lm当必有独立电感。','改变Lr会同时影响哪些特征？先说两项。',25,'A','R01 §1.2；R04 §1.3'],
 ['解释ir、im和传能电流','ip=ir−im、瞬时值与整流','在稳态截取一周期ir/im/ip；标方向和相交点；只解释观察到的区间。','自己的周期标注；能指出副边传能结束需哪些证据。','ir非零而ip=0时，能量一定继续到输出吗？',50,'B','R04 §3'],
 ['辨认实际副边整流路径','绕组极性、二极管与体二极管','核对副边接法；SR关闭时确认模型保留正确二极管路径；标正负半周期电流。','两条实际路径；若器件模型缺路径则先标缺口。','不能按原边门极直接决定所有副边导通段的原因？',50,'B','R04 §3、§13'],
 ['计算谐振特征','fr、fp、m、Q、n定义','从当前模型读值，算fr/fp与Lm/Lr；对照教学定义；负载折算确认接法。','可复算表；模型值与实物值分开；不把fp当fmin。','Lr和Cr同时翻倍，fr如何变化？',50,'B','R04 §4'],
 ['读一张有条件的FHA曲线','基波近似、阻抗、增益与工作区','按讲义统一定义算fr附近两点；同时检查Zin相位；写近似假设。','一张数值/曲线对照及限制；不把FHA当启动或闭环模型。','增益大是否自动代表感性区和ZVS？',75,'B','R04 §5–7'],
 ['做一个邻近开环对照','控制量、负载与静态灵敏度','在模型现有有效范围内选两个邻近频率或脉宽点；只改一个量；保存稳态数据。','输入、输出变化和方向有证据；理想电池钳位效应单独解释。','输出电压被电池钳住时，应再看哪个量判断传能变化？',75,'B','R03 T03；R04 §17'],
 ['识别ZVS的证据边界','Coss换流、电流方向、死区电荷','对照Vds/Vgs/ir与SW；核对是否建器件电容；列真实ZVS还缺什么。','能区分设置死区与实际完成换流；无Coss模型不声称验证ZVS。','轻载、高频时为何可能有死区仍失去ZVS？',50,'B','R04 §7；R02 §2.3']
 ]},
 {id:'S05',name:'采样链与实时执行',gate:'能追一条物理信号到控制输出的链，并给出单位、更新率和延迟依据。',tasks:[
 ['追一条采样电路','传感器、分压/放大、ADC量程','选Ibat或Vo；从图到引脚和变量；列ADC基准、增益、零点未知项。','一张信号链和量程依据；不猜器件参数。','ADC码不变，是否表示没有新采样？',50,'A','R04 §8；R11'],
 ['核对换算与有效性','量纲、符号、饱和、无效数据','抽一条实际换算；手算正常/端点；PC检查越界或无效输入行为。','正常与边界计算；输入单位与DLL物理量接口不混淆。','无效采样能否直接当作真实0V或0A？',50,'B','R11；R09；R04 §8'],
 ['量化滤波延迟','一阶滤波、时间常数、阶跃响应','选0.999/0.001等实际滤波；由Ts算时间常数；跑阶跃并看门限延迟。','计算与序列响应对照；Ts来源明确。','20拍条件计数与25ms滤波时间常数是否同一件事？',50,'B','R07 SampleLpfHandle；R02 §7.4'],
 ['还原快慢任务时间轴','中断、分频、控制周期、DLL调用','分别画MCU与DLL采样到输出路径；核对25μs/1ms/5ms来源，不从变量名推断。','两张或一张对照时间轴；未知实际执行时间单独列出。','变开关频率是否必然改变控制更新周期？',50,'A','R11 HandleFast；R08；R09'],
 ['识别共享数据一致性','快照、写入者、命令更新','追一个慢环到快环请求；列谁写、谁读、何时有效；设计半更新/过期检查。','一个可复现PC输入序列或一致性检查；不高速打印堵住快环。','新目标配旧限值能否产生与完整请求不同的动作？',50,'B','R09；R06 P1；R04 §16'],
 ['核对时序约束与输出权','最小周期、比较边界、保护优先级','对最短/最长合法周期做边沿检查；核对谁最终写PWM；测试非法配置被拒绝的规定行为。','一组合法/非法时序检查；若现有实现缺校验则记缺口。','故障与新PWM同拍到达，哪个动作必须优先？',50,'B','R08 SHRTIMERdrive；R10；R04 §9']
 ]},
 {id:'S06',name:'同步整流原理与实现',gate:'能把SR窗口放到实际副边电流上，解释许可、查表、延迟和反向风险。',tasks:[
 ['理解SR收益与双向导电','导通损耗、体二极管与MOSFET双向通路','比较二极管与MOSFET导通模型；画一条正向与一条可能反向路径；限定实际接法。','能解释何时减少损耗、何时错误导通放行反灌。','为什么SR多开一点不总是更好？',25,'B','R04 §13.1'],
 ['建立SR关闭的电流基线','整流导通区与零电流附近','先确认模型二极管路径；固定原边工况；保存副边电流/Vds及原边门极一周期。','基线含极性和时间窗；SR关断下的损耗/能力仅限模型条件。','SR关掉是否允许实机继续任意满载？',50,'B','R04 §13、§17'],
 ['核算SR延迟预算','检测、路由、PWM更新、驱动关断','从实际实现列延迟链；未知值留空；用明确的教学延迟算例解释提前关断。','延迟表含来源与不确定项；命令时刻与实际导通区分开。','指令正好在电流零点关断，真实器件会发生什么？',50,'B','R04 §13.3'],
 ['追PWM模式SR窗口','许可、Sr_Atime、Sr_Dtime与滤波','选模式5一个工况；追许可和Timer D窗口；比较硬件分支与DLL分支，不先假定完全一致。','一张窗口数值表与代码分支对照；不一致记具体位置。','平均电流满足许可，窗口仍过短时代码做什么？',75,'B','R07 PwmSyncDrvUpdate；R08；R12'],
 ['追PFM模式SR查表','频率/电流轴、选点、缩放和分支','选模式7一个点；手工追Sr_compute与mathTimeHandle；核对102kHz附近不同处理。','查表输入、索引、输出、单位和窗口均可追；表依据未知不编造。','102kHz两侧是否能只沿用同一组SR边沿？',75,'B','R12 Sr_compute/mathTimeHandle；R08'],
 ['核对SR许可与禁止','滞回、滤波、最小窗口、停机优先级','用PC/模型输入序列穿越电流门限；观察许可、计算窗口和最终门极；测试原边禁止。','门限上下与停波序列记录；不能只看SynDrv一个变量。','SynDrv=1是否证明两只SR实际导通且方向正确？',50,'B','R07；R12；R08'],
 ['做一个SR边界对照','轻载、工作区、预偏置与反向电流','先选轻载与正常点比较窗口；在模型中分析延迟关断反例；保留原版，不改实机。','电流/门极/Vds对照；未建寄生与延迟明确标缺失。','早关与晚关分别可能增加什么损耗或风险？',75,'B','R04 §13.4；R03 T05–T06']
 ]},
 {id:'S07',name:'控制器实现与局部动态',gate:'算法序列可复算；控制方向、周期、限幅和局部对象明确；设计依据与实现正确分开。',tasks:[
 ['确定控制对象与方向','误差符号、执行量、工作区','选当前实际CC函数；标反馈/给定/误差及Hz或Duty单位；用开环邻近点验证方向。','一条有工作区前提的反馈链；没有无条件频率规则。','误差换成另一种定义，PI符号还能照抄吗？',50,'B','R07 ConCurrHandle/ConPfmHandle；R04 §11.1'],
 ['复算位置与增量PI','状态递推、Kp/Ki、Ts与缩放','对实际用到的两种形式各算短误差序列；对照先积分/后输出及历史字段。','逐拍误差、内部状态、限幅前后输出一致。','代码字段叫integral就一定存的是积分量吗？',75,'B','R07 Compensator_Pid/Compensator_PidPwm；R04 §11'],
 ['检查饱和与重新接管','抗积分累积、限幅和初始化','保持误差使输出饱和，再反向；测试禁用后启用；记录现有行为与要求缺口。','可重跑序列和恢复过程；不把输出限幅当自动抗饱和。','未活动控制器长时间积分后接管会怎样？',50,'B','R07；R04 §11、§12'],
 ['核对参考斜坡与限制','A/拍、A/s、功率/温度限制','追RefRampPwmCurr与RefRampPfmCurr；换算最大斜率；比较请求与实际可用上限。','斜率和限幅依据表；条件跟踪与纯固定斜坡分开。','Ts改变后每拍步长不变，A/s是否改变？',50,'B','R07 RefRamp；R01 §2'],
 ['获取一个局部动态响应','静态灵敏度、阶跃、动态对象','在已建立的模型工作点做小扰动；记录控制量到输出响应；检查扰动幅度与稳态窗口。','一组局部响应；说明模型假设与尚缺频响依据。','静态增益曲线能否直接给PI带宽与裕度？',75,'B','R04 §10；R06 T23'],
 ['建立候选补偿依据','带宽/裕度、延迟、离散化和系数责任','在适用对象或独立教学对象上检查候选补偿；列Ts/缩放/限幅；真实设计依据不足保留缺口。','一个有对象和约束的候选分析；不声称唯一还原公司设计。','只给PI系数，能唯一恢复原作者设计模型吗？',100,'B','R04 §10–11；R06 T24'],
 ['验证限定工况闭环','跟踪、扰动、饱和与适用范围','优先接已有控制；测一个给定变化、一个负载变化和饱和恢复；先核对测量/执行。','输入与指标来源明确；不以不振荡一项判稳定设计完成。','输出不足时为何应先查限幅和实际门极？',100,'B','R04 §11、§17；R03 T07']
 ]},
 {id:'S08',name:'CC/CV与模式协同',gate:'能解释当前控制权、请求和反馈；交接时参考、PI、原边与SR分别可核验。',tasks:[
 ['识别模式与请求所有者','0～8模式、慢环命令与自动请求','列当前枚举到函数；追强制CtrMode与RequestMode的作用；分清模型强制和实机入口。','模式表对应源码与模型接口；能解除强制以观察自动链。','一直钉模式5，自动5→8→6→7还能正常被观察吗？',50,'A','R07 PowerCtrHandle；R09；R13'],
 ['核对CC/CV控制权','输出CC、CV、选择与串级','按实际代码画请求/反馈/执行量；查谁当前控制，未活动PI怎样处理。','控制权表；没有因为看到两个PI就假定串级或取min。','两路都输出频率时，照抄取最小值是否可靠？',50,'A','R07；R04 §12'],
 ['核对请求与功率约束','BMS/PMS、单位、有效期和降额','选一个外部电流请求；追限流/功率/温度路径；标1600W目标与10A/16A代码限制的差别。','请求到可执行目标的表；产品规格未知单独列出。','同样1600W，在不同电池电压下需要相同电流吗？',50,'B','R07 RefRamp；R09；R01 §2'],
 ['观察Hold与Transition','输出FIR、累计计数、短脉冲过渡','选择已支持的模型场景；追5→8→6→7；观察模式、参考、频率、比较窗和SR。','边界前后波形与状态；单看Duty不足的原因明确。','切模式同拍Duty=0.05，是否说明实际发5%PWM？',75,'B','R02 §7；R07 ConPwmHoldHandle/ConTransHandle'],
 ['核对交接内部状态','PI预置、历史误差、参考和能量','针对一处切换列进入/退出的状态修改；计算首拍输出是否匹配；测试阈值附近变化。','交接表及首拍计算；允许电流缺口/过冲指标未定则列待确认。','Ibat_FIR小能否证明Ir和Cr能量都已小？',75,'B','R02 §7；R04 §12.3'],
 ['复跑CC与CV各一个场景','电池钳位与电阻/电子负载差别','使用对应负载分别验证CC/CV；记录实际给定、上限和控制权；保留基线。','两条有负载与模式说明的记录；不拿更高电压源钳位结果误判CV。','测30V CV时接32V理想电池，为什么不能只看Vo判环路？',75,'B','R01 §1.3；R13']
 ]},
 {id:'S09',name:'软启动、停机与重启',gate:'区分四种预充；状态有完成/超时/失败出口；覆盖长停波和残余状态。',tasks:[
 ['区分四种预充','母线、自举、Cr初态、输出预充','打开02文档；在实际图标各充电路径/目标/完成条件；未知路径不要补画。','四项表；能解释已有输出预充不证明自举和Cr已准备。','独立开低管是否自动把Cr充到Vbus/2？',25,'A','R02 §1–4'],
 ['设计自举核验项','有效Cboot、电荷预算、UVLO与刷新','按驱动实际参数或待确认项建立电荷预算；列首次和0.5s停波后入口；不凭延迟定充电时间。','参数/公式/测量需求表；没型号时只完成核验设计。','充电脉冲长于传播延迟，为什么仍可能充不够？',75,'B','R02 §3；R10'],
 ['分析Cr与谐振残态','冷启/热启、首拍应力、磁通残留','在模型保留真实物理状态做热重启；选择一个残压反例；看Ir/Vcr与首拍。','初态、操作、峰值和限制记录；没有重置物理状态掩盖风险。','故障截断后软件清零能否清掉电路储能？',75,'B','R02 §2、§4'],
 ['核对输出预充与继电器','双边压差、吸合延迟、方向与超时','追SoftCurStart一条路径；列Vrly/Vbat判据、计数和继电器命令；区分软件状态/触点反馈。','动作与判据表；单边合闸、符号和60V边界列待确认。','Vrly<Vbat+0.2为何不一定表示两侧接近？',75,'A','R07 SoftCurStart；R08 ChargeOn；R02 §5'],
 ['验证一个启动失败出口','阶段超时、无法抬压、采样异常','先在PC/模型选一个已定义或待确认异常；记录现有停留/退出；要求缺失不自行当通过。','故障时间线；当前代码事实与建议处理分开。','没有达到目标时无限增加启动能量会有什么问题？',75,'B','R02 §8–9；R04 §14'],
 ['核对正常停机与快速故障','减流、封锁、续流、继电器动作','分别追普通停止与严重故障；标动作时间尺度和残能路径；先做逻辑序列核对。','两条不同出口及优先级；实机保护能力未测明确标注。','严重过流能否等待参考缓降到0再关波？',50,'B','R11；R02 §8–9'],
 ['检查重复启动的软件历史','静态计数、PI、滤波、SR状态','列preNum/preChargeTime/delay/PI等持久状态；在每次打断后重跑相关入口。','重置清单和一个重复运行对照；不把局部memset当全状态复位。','重新开始仿真一定会清除DLL内部所有static变量吗？',75,'B','R07 PowerIniPidVar/启动函数；R09；R02 §10']
 ]},
 {id:'S10',name:'故障链与异常验证',gate:'每个选定故障都有检测源、延迟、动作、锁存和恢复；模型与硬件证据分开。',tasks:[
 ['建立保护分工矩阵','硬件快保护、周期控制、慢管理','选原边过流/过温/通信超时三类；列来源、阈值依据、最坏延迟和执行者。','三条完整故障链；不同故障没有统一套连续三次。','CPU卡死时哪条保护仍必须能动作？',50,'A','R04 §15；R02 §10'],
 ['区分原边与输出过流','循环电流、继电器条件、快检测','追SwOCP与HRTIM Fault配置；标RelayOld条件与PLECS_DLL屏蔽；列原边采样缺口。','代码/硬件路径表；DLL通过不被当作实机过流验证。','Ibat=0但Ir很大，输出软件限流能单独兜住吗？',50,'A','R11；R10；R02 §10'],
 ['验证反馈与命令异常','无效、过期、符号、量程和通信','选一条采样或请求，注入失效序列；观察积分、模式与输出许可。','异常输入和预期/实际动作记录；数据不可信没有被静默当零。','数值未变与旧报文重放，如何区分？',75,'B','R04 §8、§16；R06 P1/P3'],
 ['核对复位和故障优先级','复位默认态、看门狗、门极封锁','先纸面/PC核对故障与控制同时到达、复位及调试暂停的设计要求；列逻辑测试计划。','事件优先级表及缺失依据；未做MCU测试不标C。','程序显示OFF是否等于浮动高侧MOSFET已关？',50,'A','R10；R04 §15'],
 ['分析执行器异常','继电器拒动/粘连、泄放与SR误开','选择一个相关故障建行为模型或分析路径；记录如何识别并禁止继续升功率。','一个可复现异常或明确未建项；不把理想软件继电器当真实触点。','继电器粘连时泄放会不会持续耗电池？',75,'B','R02 §9；R04 §13–15'],
 ['定义恢复与重试约束','锁存、复位权限、冷却和残态','对上面三类故障分别写恢复条件、重试次数/间隔来源和重新准入；未规定值列待确认。','恢复表加一条异常重入序列；严重故障没有无条件无限重试。','故障撤销是否自动意味着可恢复大功率输出？',50,'B','R02 §8–9；R04 §15.2']
 ]},
 {id:'S11',name:'PFC与充电机接口',gate:'理解前级基本功率路径和两环职责，能解释母线/功率/命令如何约束LLC。',tasks:[
 ['识别图腾柱PFC功率路径','快慢桥臂、整流、升压与母线','对照实际图标交流输入、桥臂、电感、母线及预充；先学正常半周路径。','一张PFC功能图；不把LLC输入范围当整机AC范围。','380～420V母线能推出整机最低AC输入吗？',50,'A','R01；R06 §7.3'],
 ['理解PFC控制分工','母线外环、输入电流内环与参考','追现有PFC接口或公开定义；标测量与执行量；区分两环与充电CC/CV。','控制分工图和当前可核实函数/库接口；黑盒内部未知。','PFC两环与LLC的CC/CV是不是同一概念？',75,'A','R06 §7.3；R01'],
 ['认识过零与PFC运行阶段','交流极性、桥臂交接、就绪状态','从代码/已有波形找一个过零或启动阶段；标许可、状态和所需验证信号。','一段时序或待测清单；不在陌生实机尝试过零参数。','母线暂时到400V是否证明PFC已可持续供额定功率？',50,'A','R06 P3；实际PFC源码/图'],
 ['建立PFC与LLC运行契约','PFC_ok、Vbus、掉电与故障传播','追就绪/撤销到LLC许可；列母线反馈、恢复稳定条件和停机责任；做一个接口序列。','请求/实际状态表；PFC_ok没有被当作Vbus测量值。','LLC快DLL没有Vbus输入时，能验证母线OVP吗？',75,'B','R11；R09；R01 §2'],
 ['核对整机功率降额','AC能力、温度、输出电压和电流上限','将当前代码限值与规格表对齐；算几个Vbat×I点；列连续/峰值、温升和PFC能力未知项。','工作点功率表及来源；没有用项目名称替代能力曲线。','为何30V与60V不一定都可持续1600W？',50,'B','R07 RefRampPfmCurr；R01 §2'],
 ['核对整机请求与停止接口','BMS/PMS、超时、更新与控制许可','追一个请求到快环及一个通信超时；在允许环境做PC/接口序列，核对快照与优先级。','一条端到端序列；升级/故障时输出互锁列明。','旧充电请求在复位后保留，是否应直接恢复？',75,'B','R09；R06 P3']
 ]},
 {id:'S12',name:'验证、交付与复用',gate:'同版本证据可复跑；通过评审的逻辑/实机范围明确；能交付负责链路并复用。',tasks:[
 ['建立参数与依据登记','单位、Ts、版本、范围、标定与设计来源','整理当前实际用到的关键参数；每项填来源、用途、允许变更和未知；复核计算书缺口。','一张参数登记表；PI/SR原设计理由未知没有被编造。','改变Ts或磁件版本后，哪些参数必须重核？',75,'A','R01；R06 T24/T34'],
 ['形成分层回归矩阵','需求到逻辑、模型、时序和实机证据','汇总已有记录；按母线/电池/负载/初态/模式选规定覆盖；记录未测而非一律通过。','需求—用例—判据—证据关系；一个已修问题有回归项。','只测最终Vo达标为什么不足以验收启动？',75,'B','R02 §11；R04 §18'],
 ['完成一次有证据的定位','预测、排除、根因与修复范围','选已有模型/代码差异；先列两种解释；做一个能区分的检查；保留失败候选。','复现、证据、结论与回归；根因不足则保持未确认。','第二个AI赞同你的判断，能否作为根因证据？',100,'B','R03 §5、§10；R06 T36'],
 ['完成MCU逻辑时序验证','周期/边沿、ADC、ISR、Fault与首拍','准备匹配开发板和测试配置；功率级断开，按项目安排测一组时序和故障封锁。','实际波形、版本、事件关系及测量条件；没有设备则资源待补。','逻辑时序通过是否已经证明高压ZVS和SR功率行为？',100,'C','R05 L4；R10'],
 ['完成受指导的功率台架核验','Vgs/Vds/Ir/Vcr、驱动和功率边界','先提交一个具体测试单给项目负责人；按台架流程取得对应工作点及必要异常证据。','实际数据和验收范围；自举、SR、保护未测项独立标注；无资源不以模型替代。','某工作点成功为何不能宣称所有母线/负载通过？',120,'D','R05 L6；R02 §11'],
 ['交付并在第二次任务中复用','构建、版本、评审、发布和责任边界','交付负责链路的需求/代码/证据/限制；完成真实评审或发布记录；在下一任务复用一个检查。','实际职责、验收/发布记录和复用证据；个人练习不冒充产品发布。','换硬件版本时哪些旧结论可复用、哪些必须重新测？',100,'E','R06 P3；R04 §18']
 ]}
];

const tasks=[];
for (const s of stages) s.tasks.forEach((t,i)=>tasks.push({id:`${s.id}-${String(i+1).padStart(2,'0')}`,stage:s.id,stageName:s.name,t}));
const n=tasks.length, first=7, last=first+n-1;
if(n!==76) throw new Error(`Unexpected task count ${n}`);
const known=[
 ['S01','项目边界','区分AC整机、DC/DC、继电器前后测量与已有资料的版本。','能画四条链；未知不猜填。','S01-01～06','版本表＋开环基线'],
 ['S01','证据与共识','哪些是用户确认、代码事实、模型结果或设计解释？','能给自己的结论标前提和未验证项。','S01-02、06','记录＋闭卷变式'],
 ['S02','L/C初值与能量','同一个脉冲，为什么初态不同会得到不同峰值？','独立计算di/dt、dv/dt和储能，带单位与初值。','S02-01～03、05','手算＋单位检查'],
 ['S02','变压器与负载折算','Np/Ns和绕组极性怎样定义？','确认实际整流接法；不混用半绕组/全绕组公式。','S02-04','图＋折算例'],
 ['S02','RMS与阻抗','平均电流、损耗和相位为什么不能混算？','能算简单RMS及复数串并联。','S02-06','独立算例'],
 ['S03','原边门极','模式、频率、Duty和许可分别做什么？','算两种模式窗口；知道Duty并非通用互补占空比。','S03-01～04','边沿表＋模型门极'],
 ['S03','驱动与第一拍','高侧供电从哪里来，PWM怎样真正生效？','找到型号/供电路径；说明预装载、GPIO及Fault的关系。','S03-05～06','路径和事件表'],
 ['S04','谐振与传能','Lr/Lm/Cr、ir/im/ip各表示什么？','能标一周期导通段及传能证据。','S04-01～03','周期图＋电流方向'],
 ['S04','工作区与近似','fr、fp、FHA和静态扫点各能说明什么？','统一m/Q/n定义；不从增益或fr直接批准频率边界。','S04-04～06','计算＋邻近点对照'],
 ['S04','软开关','设置死区为什么不一定有ZVS？','联系Coss、电流方向、Vds/Vgs与模型缺项。','S04-07','换流证据与限制'],
 ['S05','可信测量','真实电流/电压怎样变成软件值？','找到量程/零点/增益/方向；测试无效与越界。','S05-01～02','信号链＋数值核验'],
 ['S05','时间与一致性','滤波、控制周期、PWM更新和数据版本怎样影响控制？','能给一条链建立有来源的时间轴。','S05-03～06','序列/时序检查'],
 ['S06','SR物理作用','导通后为什么能反向，关断后为什么仍可能续流？','确认实际副边通路；解释早关损耗与晚关风险。','S06-01～03','路径＋关断延迟预算'],
 ['S06','SR算法','许可、查表窗口与器件实际导通有什么区别？','追一个PWM点和一个PFM点，核对轴/索引/单位。','S06-04～06','数值窗口表'],
 ['S06','SR边界','轻载、变频、预偏置为何不能固定半周导通？','有一项边界反例；实机延迟未知仍保留。','S06-07','波形与范围记录'],
 ['S07','调节器运算','位置/增量PI、Ts和历史状态怎样对应代码？','手算短序列；解释限幅和接管首拍。','S07-01～04','PC/计算对照'],
 ['S07','控制设计依据','静态趋势为何不能给出闭环裕度？','明确局部对象、延迟、指标、离散化和参数责任。','S07-05～07','局部动态/补偿分析'],
 ['S08','充电控制权','当前谁控制功率，BMS/PMS目标如何受限？','分清CC/CV结构及请求/实际目标。','S08-01～03','控制权和约束表'],
 ['S08','交接连续性','参考、PI、原边和SR怎样在模式边界协调？','共同看模式、边沿和状态；不只看Duty。','S08-04～06','交接时间线'],
 ['S09','启动初态','四种预充分别准备什么？','区分自举、Cr和输出；覆盖长停波与热重启。','S09-01～03','路径/电荷预算/初态实验'],
 ['S09','状态与恢复','预充、合闸、缓升、停机各怎样完成和失败？','每阶段有动作、判据、超时或明确缺口。','S09-04～07','状态与重置表'],
 ['S10','保护责任','什么必须快关，什么允许滤波或慢降额？','三类完整故障链，含锁存和恢复。','S10-01～02、06','故障矩阵'],
 ['S10','异常证据','采样、命令、复位、执行器失效如何影响门极？','完成一个反例；CPU不参与的路径独立验证。','S10-03～05','输入序列与事件表'],
 ['S11','PFC基本对象','快慢桥臂、两环、过零和母线分别是什么？','懂功能与接口；黑盒内部不声称已掌握。','S11-01～03','功能/控制分工图'],
 ['S11','整机约束','PFC_ok、Vbus、功率上限和请求如何传到LLC？','能解释准入、撤销、掉电与目标限制。','S11-04～06','端到端接口序列'],
 ['S12','复现与交付','如何证明修复、回归和适用范围？','版本、原始数据、判据和责任可追踪。','S12-01～03、06','可重跑交付包'],
 ['S12','实机证据','逻辑、模型与功率台架分别证明什么？','C/D/E证据分别获得；资源不足明确待补。','S12-04～06','逻辑波形/功率数据/验收记录']
];

const wb=Workbook.create();
const home=wb.worksheets.add('今日任务');
const phase=wb.worksheets.add('阶段计划');
const plan=wb.worksheets.add('任务总表');
const know=wb.worksheets.add('必懂清单');
const logs=wb.worksheets.add('实验记录');
const spec=wb.worksheets.add('项目规格与资料');
const colors={navy:'#183B56',blue:'#EAF1F8',text:'#243746',muted:'#587082',input:'#FFF1CC',green:'#E5F1E9',red:'#FCE7E5',line:'#CFDAE3'};
function base(s,range){s.showGridLines=false;s.getRange(range).format.font={name:'Arial',size:11,color:colors.text};s.getRange(range).format.verticalAlignment='center';}
function title(s,text){s.getRange('B2').values=[[text]];s.getRange('B2').format.font={name:'Arial',size:16,bold:true,color:colors.navy};}
function head(s,r){s.getRange(r).format={fill:colors.navy,font:{name:'Arial',size:11,bold:true,color:'#FFFFFF'},wrapText:true,horizontalAlignment:'center',verticalAlignment:'center',rowHeight:32};}
function widths(s,arr){arr.forEach(([c,w])=>s.getRange(`${c}1:${c}120`).format.columnWidth=w);}
function lightRows(s,firstRow,lastRow,lastCol){for(let r=firstRow;r<=lastRow;r++)if(r%2===0)s.getRange(`B${r}:${lastCol}${r}`).format.fill='#F3F6F9';}
function setInput(s,range){s.getRange(range).format.fill=colors.input;s.getRange(range).format.font.color='#76530D';}
const q=s=>`'${s}'`;
const pr=(col)=>`${q(plan.name)}!$${col}$${first}:$${col}$${last}`;

base(plan,`A1:V${last}`);title(plan,'顺序任务与完成证据');
plan.getRange('B3').values=[['从上到下每次做一项。已有能力用证据快速核验；黄色列由你填写。']];
plan.getRange('B4').values=[['完成判定：状态完成＋证据＋复述＋变式＋证据等级＋前置通过。起步用时不含台架排期、补课和返工。']];
const headers=['序号','任务ID','阶段','唯一问题','必须知道','具体操作','交付与通过条件','变式或反例','起步分钟','前置ID','证据目标','资料入口','状态','实际分钟','证据位置','闭卷复述','变式核验','实际证据','结论或卡点','完成日期','任务判定','证据匹配'];
plan.getRange('A6:V6').values=[headers];head(plan,'A6:V6');
const rows=tasks.map((o,i)=>[i+1,o.id,o.stage,o.t[0],o.t[1],o.t[2],o.t[3],o.t[4],o.t[5],i?tasks[i-1].id:'',o.t[6],o.t[7],'未开始',null,'','未做','未做','','',null,null,null]);
plan.getRange(`A${first}:V${last}`).values=rows;
plan.getRange(`D${first}:L${last}`).format.wrapText=true;
plan.getRange(`O${first}:O${last}`).format.wrapText=true;
plan.getRange(`S${first}:S${last}`).format.wrapText=true;
plan.getRange(`A${first}:V${last}`).format.rowHeight=64;
lightRows(plan,first,last,'V');
setInput(plan,`M${first}:T${last}`);
widths(plan,[['A',6],['B',12],['C',9],['D',27],['E',31],['F',53],['G',48],['H',43],['I',11],['J',12],['K',10],['L',32],['M',13],['N',11],['O',32],['P',12],['Q',12],['R',12],['S',40],['T',15],['U',19],['V',16]]);
plan.getRange(`I${first}:I${last}`).setNumberFormat('0');plan.getRange(`N${first}:N${last}`).setNumberFormat('0');plan.getRange(`T${first}:T${last}`).setNumberFormat('yyyy-mm-dd');
plan.getRange(`M${first}:M${last}`).dataValidation={rule:{type:'list',values:['未开始','进行中','待复核','完成','受阻','已会待核验']}};
plan.getRange(`P${first}:Q${last}`).dataValidation={rule:{type:'list',values:['未做','通过','需补']}};
plan.getRange(`R${first}:R${last}`).dataValidation={rule:{type:'list',values:['A','B','C','D','E']}};
plan.getRange(`N${first}:N${last}`).dataValidation={rule:{type:'decimal',operator:'greaterThanOrEqual',formula1:0}};
for(let r=first;r<=last;r++){
 plan.getRange(`V${r}`).formulas=[[`=IF(R${r}="","待填证据等级",IF(MATCH(R${r},'项目规格与资料'!$B$40:$B$44,0)>=MATCH(K${r},'项目规格与资料'!$B$40:$B$44,0),"达标","证据不足"))`]];
 const prereq=r===first?'"通过"':`U${r-1}`;
 plan.getRange(`U${r}`).formulas=[[`=IF(M${r}<>"完成","未通过",IF(O${r}="","缺证据位置",IF(P${r}<>"通过","待闭卷复述",IF(Q${r}<>"通过","待变式核验",IF(V${r}<>"达标",V${r},IF(${prereq}<>"通过","前置未通过","通过"))))))`]];
}
plan.getRange(`U${first}:U${last}`).conditionalFormats.addCustom(`=$U${first}="通过"`,{fill:colors.green,font:{color:'#285D3D'}});
plan.getRange(`U${first}:U${last}`).conditionalFormats.addCustom(`=AND($M${first}="完成",$U${first}<>"通过")`,{fill:colors.red,font:{color:'#9D2920'}});
plan.getRange(`M${first}:M${last}`).conditionalFormats.add('containsText',{text:'受阻',format:{fill:colors.red,font:{color:'#9D2920'}}});
plan.tables.add(`A6:V${last}`,true,'LearningTasks');plan.freezePanes.freezeRows(6);plan.freezePanes.freezeColumns(4);

base(spec,'A1:I72');title(spec,'项目事实、待确认项与资料入口');
spec.getRange('B3').values=[['当前确认值、模型值、代码值与未来项目确认值分别保存。黄色列补充依据，不覆盖原事实。']];
const specRows=[
 ['前级','图腾柱无桥PFC',null,'','用户确认','','','R01；本次对话'],
 ['母线正常电压','正常工作值',400,'V','用户确认',null,'','R01；本次对话'],
 ['母线最低工作电压','范围下限',380,'V','用户确认',null,'','R01；本次对话'],
 ['母线最高工作电压','范围上限',420,'V','用户确认',null,'','R01；本次对话'],
 ['电池最低电压','工作范围下限',30,'V','用户确认',null,'','R01；本次对话'],
 ['电池最高电压','范围上限；非统一CV目标',60,'V','用户确认',null,'','R01；本次对话'],
 ['项目功率目标','覆盖工况与连续/峰值待定',1600,'W','项目目标',null,'','R01；本次对话'],
 ['最大连续电流','待确认',null,'A','未知',null,'','R01 §2'],
 ['电池CV与充电策略','电池类型、串数、BMS目标待确认',null,'V','未知',null,'','R01 §2'],
 ['整机AC范围','不能从DC母线范围推出',null,'V rms','未知',null,'','R01 §2'],
 ['当前原理图版本','用户有完整原理图；匹配情况待填',null,'','用户陈述','','','S01-01'],
 ['当前代码/DLL版本','已有真实代码与DLL；二进制匹配待填',null,'','用户陈述','','','S01-01'],
 ['开环模型能力','能独立运行并查看波形',null,'','用户确认','','','2026-10-09用户回复'],
 ['当前开环文件与负载','具体文件、负载及调制待记录',null,'','未知','','','S01-04'],
 ['Lr','读取过的FullTest模型',20,'μH','模型值',null,'','R01 §1.2；当前文件需核对'],
 ['Cr','读取过的FullTest模型',164,'nF','模型值',null,'','R01 §1.2；当前文件需核对'],
 ['Lm','读取过的FullTest模型',100,'μH','模型值',null,'','R01 §1.2；实物待核'],
 ['绕组参数','读取过的FullTest：[18 4 4]',null,'匝/模型参数','模型值','','','R01 §1.2；Ns定义待核'],
 ['输出Co','读取过的FullTest模型',100,'μF','模型值',null,'','R01 §1.2；当前文件需核对'],
 ['原边驱动与自举','型号、Cboot、VDD、UVLO待确认',null,'','未知','','','R02 §3'],
 ['SR表依据与延迟','代码含查表；采集/设计依据未知',null,'','代码事实','','','R12；R02'],
 ['保护与台架资源','Fault配置不等于已实测；权限/设备待填',null,'','待确认','','','R10；S12-04～05'],
 ['PWM电流参考上限','当前代码路径值；非产品最大能力',10,'A','代码值',null,'','R07 RefRampPwmCurr'],
 ['PFM电流上限','当前CON_CURR_OUT路径值',16,'A','代码值',null,'','R01 §2；当前版本需核对']
];
spec.getRange('B5:I5').values=[['参数或资料','当前事实','数值','单位','证据类型','你确认的项目值','依据或未知原因','来源']];head(spec,'B5:I5');
spec.getRange(`B6:I${5+specRows.length}`).values=specRows;
spec.getRange('B6:I29').format.wrapText=true;spec.getRange('B6:I29').format.rowHeight=44;lightRows(spec,6,29,'I');setInput(spec,'G6:H29');
spec.getRange('D6:D29').setNumberFormat('0');
spec.getRange('B32').values=[['使用方法：先填当前任务需要的参数；不知道就写未知及应找的资料，不等全部填满。']];
spec.getRange('B34').values=[['计算书缺口：依次补参数来源、开环趋势、采样/调制延迟、局部补偿、SR窗口与启动条件。']];
spec.getRange('B38:D38').values=[['证据等级','实际完成的验证','不能代替']];head(spec,'B38:D38');
spec.getRange('B40:D44').values=[['A','资料/代码/图追踪','模型或实机通过'],['B','手算/PC测试/明确模型','MCU实际时序与功率行为'],['C','功率级断开的MCU逻辑实测','高压换流、ZVS和效率'],['D','规定工况的实际功率台架','未测工况与完整量产'],['E','负责范围的真实交付/发布','他人设计与全部整机职责']];
spec.getRange('B40:D44').format.wrapText=true;spec.getRange('B40:D44').format.rowHeight=45;
spec.getRange('B47:E47').values=[['资料ID','名称','使用范围','已存在的本地位置']];head(spec,'B47:E47');
const sources=[
 ['R01','系统规格与框架','节点、范围、模型/代码值','01_System_Specification.md',project],
 ['R02','软启动分析','自举、Cr、继电器、交接、失效','02_Half_Bridge_LLC_Soft_Start_Analysis.md',project],
 ['R03','项目学习工作流','单问题、证据、AI提问与当前任务','03_LLC_SR_AI_项目学习工作流.md',project],
 ['R04','半桥LLC培训讲义','基础原理与对应章节','半桥LLC_培训讲义.md',study],
 ['R05','学习与实验指导','作业、证据分级、L4/L6条件','半桥LLC_学习与实验指导.md',study],
 ['R06','项目与知识体系','P1/P2/P3及按需任务卡','电源控制软件工程师_项目与知识体系.md',study],
 ['R07','mathR02.c','快环、PI、参考、启动与交接','R02_LLC_APP_V1.1.1_0824/AppUser/mathR02.c',project],
 ['R08','ConsoleSlow.c','慢业务与SHRTIMERdrive','R02_LLC_APP_V1.1.1_0824/AppUser/ConsoleSlow.c',project],
 ['R09','快DLL包装main.c','8入17出、物理量与调用','Visual studio projects/pi_controller/pi_controller/main.c',project],
 ['R10','MCU main.c','HRTIM、Fault和初始使能','R02_LLC_APP_V1.1.1_0824/Core/Src/main.c',project],
 ['R11','ConsoleFast.c','快调度、许可与软件保护','R02_LLC_APP_V1.1.1_0824/AppUser/ConsoleFast.c',project],
 ['R12','Sr_Handle.c','SR表、系数与时序处理','R02_LLC_APP_V1.1.1_0824/AppUser/Sr_Handle.c',project],
 ['R13','各模式测试手册','定位用例；旧结论需核对','LLC各模式PLECS测试手册.md',project],
 ['R14','唯一执行方案','小闭环及资源边界；旧日程不重复执行','电源控制软件工程师_最终执行方案.md',study]
];
for(const [id,name,scope,file,dir] of sources){await fs.access(path.join(dir,file));}
spec.getRange(`B48:E${47+sources.length}`).values=sources.map(([id,name,scope,file,dir])=>[id,name,scope,`${dir}/${file}`]);
spec.getRange(`B48:E${47+sources.length}`).format.wrapText=true;spec.getRange(`B48:E${47+sources.length}`).format.rowHeight=60;
widths(spec,[['A',3],['B',26],['C',42],['D',15],['E',60],['F',17],['G',28],['H',42],['I',44]]);
spec.freezePanes.freezeRows(5);spec.freezePanes.freezeColumns(2);

base(phase,'A1:J24');title(phase,'阶段顺序与放行条件');
phase.getRange('B3').values=[['先核验已有能力。每阶段通过后再进入下一阶段；可预习，但不跳过证据。']];
phase.getRange('B4').values=[['离线任务先推进。C/D/E阶段需对应设备、台架与真实职责，资源不足标受阻，不用仿真替代。']];
phase.getRange('B6:J6').values=[['阶段','学习范围','任务数','通过数','进度','起步小时','预算学习周','阶段放行条件','判定']];head(phase,'B6:J6');
for(let i=0;i<stages.length;i++){
 const r=7+i,s=stages[i];phase.getRange(`B${r}:J${r}`).values=[[s.id,s.name,null,null,null,null,null,s.gate,null]];
 phase.getRange(`D${r}:H${r}`).formulas=[[
  `=COUNTIFS(${pr('C')},B${r})`,
  `=COUNTIFS(${pr('C')},B${r},${pr('U')},"通过")`,
  `=IF(D${r}=0,"",E${r}/D${r})`,
  `=SUMIFS(${pr('I')},${pr('C')},B${r})/60`,
  `=IF('今日任务'!$F$5>0,G${r}*60/'今日任务'!$F$5,"请填预算")`
 ]];
 phase.getRange(`J${r}`).formulas=[[`=IF(E${r}=D${r},"阶段通过","继续当前任务")`]];
}
phase.getRange('B7:J18').format.wrapText=true;phase.getRange('B7:J18').format.rowHeight=64;lightRows(phase,7,18,'J');phase.getRange('F7:F18').setNumberFormat('0%');phase.getRange('G7:H18').setNumberFormat('0.0');
phase.getRange('B21').values=[['预算学习周仅换算起步分钟，不含补课、工具、返工、评审及台架排期，不是完成承诺。']];
widths(phase,[['A',3],['B',9],['C',31],['D',10],['E',10],['F',12],['G',13],['H',15],['I',64],['J',22]]);
phase.getRange('J7:J18').conditionalFormats.add('containsText',{text:'阶段通过',format:{fill:colors.green,font:{color:'#285D3D'}}});phase.tables.add('B6:J18',true,'LearningStages');phase.freezePanes.freezeRows(6);

base(know,'A1:H40');title(know,'项目必须知道的内容');
know.getRange('B3').values=[['跟当前阶段逐项回答。不会就回到对应任务；不把整页清单当今天的阅读量。']];
know.getRange('B5:H5').values=[['阶段','知识主题','必须能回答的项目问题','最低掌握标准','对应任务','检查方式','你的解释或缺口']];head(know,'B5:H5');
know.getRange(`B6:H${5+known.length}`).values=known.map(r=>[...r,'']);
know.getRange(`B6:H${5+known.length}`).format.wrapText=true;know.getRange(`B6:H${5+known.length}`).format.rowHeight=54;lightRows(know,6,5+known.length,'H');setInput(know,`H6:H${5+known.length}`);
widths(know,[['A',3],['B',10],['C',23],['D',49],['E',52],['F',28],['G',30],['H',48]]);
know.tables.add(`B5:H${5+known.length}`,true,'MustKnow');know.freezePanes.freezeRows(5);know.freezePanes.freezeColumns(3);

base(logs,'A1:M38');title(logs,'实验与学习记录');
logs.getRange('B3').values=[['每次留一条：先预测，后运行，再比较。未运行写未运行；AI示例不算你的结果。']];
logs.getRange('B5:M5').values=[['日期','任务ID','版本与工况','我的预测或缺口','实际操作','观察结果与证据位置','差异与原因','结论及适用范围','证据等级','下一步25分钟','实际分钟','一个仍未知的问题']];head(logs,'B5:M5');
logs.getRange('B6:M35').values=Array.from({length:30},()=>Array(12).fill(null));setInput(logs,'B6:M35');logs.getRange('B6:M35').format.wrapText=true;logs.getRange('B6:M35').format.rowHeight=54;
logs.getRange('B6:B35').setNumberFormat('yyyy-mm-dd');logs.getRange('L6:L35').setNumberFormat('0');logs.getRange('C6:C35').dataValidation={rule:{type:'list',formula1:`'任务总表'!$B$${first}:$B$${last}`}};logs.getRange('J6:J35').dataValidation={rule:{type:'list',values:['A','B','C','D','E']}};
widths(logs,[['A',3],['B',15],['C',12],['D',37],['E',44],['F',42],['G',48],['H',42],['I',49],['J',11],['K',40],['L',12],['M',38]]);logs.tables.add('B5:M35',true,'StudyRecords');logs.freezePanes.freezeRows(5);logs.freezePanes.freezeColumns(3);

base(home,'A1:F38');title(home,'半桥LLC与同步整流学习计划');
widths(home,[['A',3],['B',23],['C',78],['D',3],['E',25],['F',24]]);
home.tabColor=colors.navy;phase.tabColor='#47708E';plan.tabColor='#47708E';
home.getRange('B4:C4').values=[['通过任务',null]];home.getRange('C4').formulas=[[`=COUNTIFS(${pr('U')},"通过")&" / "&COUNTA(${pr('B')})`]];
home.getRange('E4:F4').values=[['版本日期',new Date('2026-10-09T00:00:00Z')]];home.getRange('F4').setNumberFormat('yyyy-mm-dd');
home.getRange('E5:F5').values=[['每周学习预算（分钟）',120]];setInput(home,'F5');home.getRange('F5').dataValidation={rule:{type:'whole',operator:'between',formula1:25,formula2:10080}};
home.getRange('E6:F6').values=[['单次学习（分钟）',25]];setInput(home,'F6');home.getRange('F6').dataValidation={rule:{type:'whole',operator:'between',formula1:5,formula2:120}};
home.getRange('B5:C5').values=[['资源起点','你已能独立运行开环模型并查看波形；DLL已完成（用户陈述）。']];
home.getRange('B6:C6').values=[['执行规则','已会内容用复述和变式快速核验，工具安装不重复安排。']];
home.getRange('C5:C6').format.wrapText=true;home.getRange('B5:C6').format.rowHeight=44;
home.getRange('B7').values=[['自动建议任务']];home.getRange('C7').formulas=[[`=IF(COUNTIFS(${pr('U')},"通过")=COUNTA(${pr('B')}),"全部任务通过",INDEX(${pr('B')},COUNTIFS(${pr('U')},"通过")+1))`]];
home.getRange('E7:F7').values=[['手选任务ID（空为自动）','']];setInput(home,'F7');home.getRange('F7').dataValidation={rule:{type:'list',formula1:`'任务总表'!$B$${first}:$B$${last}`}};
home.getRange('B9').values=[['本次任务ID']];home.getRange('C9').formulas=[['=IF(F7="",C7,F7)']];
const get=(col)=>`IF($C$9="全部任务通过","",INDEX(${pr(col)},MATCH($C$9,${pr('B')},0)))`;
const cardRows=[
 [10,'阶段','C'],[11,'唯一问题','D'],[12,'必须知道','E'],[13,'具体操作','F'],[14,'交付与通过条件','G'],[15,'变式或反例','H'],[16,'起步用时（分钟）','I'],[17,'前置任务','J'],[18,'证据目标','K'],[19,'资料入口','L'],[20,'当前状态','M'],[21,'任务判定','U']
];
for(const [r,label,col] of cardRows){home.getRange(`B${r}`).values=[[label]];home.getRange(`C${r}`).formulas=[[`=${get(col)}`]];home.getRange(`C${r}`).format.wrapText=true;home.getRange(`B${r}`).format.font.bold=true;home.getRange(`B${r}:C${r}`).format.fill=r%2?colors.blue:'#F3F6F9';home.getRange(`B${r}:C${r}`).format.rowHeight=({11:32,12:42,13:58,14:58,15:48})[r]??30;}
home.getRange('C17').formulas=[[`=IF($C$9="全部任务通过","",IF(INDEX(${pr('J')},MATCH($C$9,${pr('B')},0))="","无",INDEX(${pr('J')},MATCH($C$9,${pr('B')},0))))`]];
home.getRange('B23:C23').values=[['每天的顺序','先预测5分钟，定点取证5分钟，操作12分钟，记录3分钟；做不完留下一步。']];
home.getRange('B24:C24').values=[['更新进度','在任务总表黄色M:T列填状态、证据、复述、变式等；任务判定和建议任务自动更新。']];
home.getRange('B25:C25').values=[['缺设备时','继续完成前面的离线任务；C/D/E任务记录资源缺口，不把模型证据升级成实机。']];
home.getRange('B26:C26').values=[['不要一次读全表','今天只做本次任务；未来工况在对应阶段再展开。首次版本/规格确认可用现有资料快速完成。']];
home.getRange('B23:C26').format.wrapText=true;home.getRange('B23:C26').format.rowHeight=44;
home.getRange('B28:C28').values=[['给AI的限定提问','我正在做任务__，工况__，自己的判断/缺口__，实际结果__。请只纠正一处并给一个变式，等我回答再继续。']];home.getRange('B28:C28').format.wrapText=true;home.getRange('B28:C28').format.rowHeight=70;
home.getRange('E9:F9').values=[['任务总数',n]];
home.getRange('E10:F10').values=[['起步总小时',null]];home.getRange('F10').formulas=[[`=SUM(${pr('I')})/60`]];home.getRange('F10').setNumberFormat('0.0');
home.getRange('E11:F11').values=[['按预算折算学习周',null]];home.getRange('F11').formulas=[['=IF(F5>0,F10*60/F5,"请填预算")']];home.getRange('F11').setNumberFormat('0.0');
home.getRange('E13:F15').values=[['范围','起步量，不是工期'],['台架/补课/返工','另计'],['起点能力','记录为已知，不代替任务证据']];home.getRange('E13:F15').format.wrapText=true;home.getRange('E13:F15').format.rowHeight=48;
home.getRange('E17:F20').values=[['填写位置','任务总表 M:T'],['黄色','可编辑输入'],['A/B证据','离线追踪/计算/模型'],['C/D/E证据','逻辑/功率/真实交付']];home.getRange('E17:F20').format.wrapText=true;home.getRange('E17:F20').format.rowHeight=42;

// Verification exercises are restored before exporting the single deliverable.
wb.recalculate();
const before=home.getRange('C7').values[0][0];
if(before!==tasks[0].id) throw new Error(`Wrong first suggestion: ${before}`);
plan.getRange('M7').values=[['完成']];wb.recalculate();
if(plan.getRange('U7').values[0][0]!=='缺证据位置')throw new Error('Completion guard failed');
plan.getRange('O7').values=[['verification-only']];plan.getRange('P7:Q7').values=[['通过','通过']];plan.getRange('R7').values=[['A']];wb.recalculate();
if(plan.getRange('U7').values[0][0]!=='通过'||home.getRange('C7').values[0][0]!==tasks[1].id)throw new Error('Progress recalculation failed');
plan.getRange('M8').values=[['完成']];plan.getRange('O8').values=[['verification-only']];plan.getRange('P8:Q8').values=[['通过','通过']];plan.getRange('R8').values=[['A']];wb.recalculate();
if(plan.getRange('U8').values[0][0]!=='证据不足')throw new Error('Evidence level guard failed');
plan.getRange('R8').values=[['B']];wb.recalculate();
if(plan.getRange('U8').values[0][0]!=='通过')throw new Error('Evidence matching failed');
plan.getRange('M7').values=[['未开始']];wb.recalculate();
if(plan.getRange('U8').values[0][0]!=='前置未通过')throw new Error('Prerequisite guard failed');
for(const r of [7,8]){plan.getRange(`M${r}`).values=[['未开始']];plan.getRange(`O${r}`).values=[['']];plan.getRange(`P${r}:Q${r}`).values=[['未做','未做']];plan.getRange(`R${r}`).values=[['']];}
home.getRange('F7').values=[[tasks[19].id]];wb.recalculate();
if(home.getRange('C9').values[0][0]!==tasks[19].id)throw new Error('Manual selector failed');
home.getRange('F7').values=[['']];
home.getRange('F5').values=[[240]];wb.recalculate();const halfWeeks=home.getRange('F11').values[0][0];home.getRange('F5').values=[[120]];wb.recalculate();if(Math.abs(home.getRange('F11').values[0][0]-2*halfWeeks)>1e-9)throw new Error('Budget recalculation failed');
wb.recalculate();
const errors=await wb.inspect({kind:'match',searchTerm:'#REF!|#DIV/0!|#VALUE!|#NAME\\?|#N/A|#NUM!|#NULL!|#SPILL!|#CALC!',options:{useRegex:true,maxResults:30},maxChars:2200});
console.log('ERROR_SCAN',errors.ndjson);
const finalChecks={tasks:n,stages:stages.length,mustKnow:known.length,firstSuggestion:home.getRange('C7').values[0][0],passed:plan.getRange(`U${first}:U${last}`).values.filter(r=>r[0]==='通过').length,startHours:home.getRange('F10').values[0][0],allSourcesExist:true};
console.log('CHECKS',JSON.stringify(finalChecks));
await fs.writeFile(path.join(outDir,'verification.json'),JSON.stringify({checks:finalChecks,errorScan:errors.ndjson},null,2));
const previews=[['今日任务','B2:F28','today'],['阶段计划','B2:J12','phases'],['任务总表','A6:H10','tasks'],['任务总表','M6:V11','tracking'],['必懂清单','B2:H11','knowledge'],['实验记录','B2:G10','records'],['项目规格与资料','B2:I12','spec'],['项目规格与资料','B38:E53','sources']];
for(const [sheetName,range,name] of previews){const blob=await wb.render({sheetName,range,scale:1.4,format:'png'});await fs.writeFile(path.join(outDir,`${name}.png`),new Uint8Array(await blob.arrayBuffer()));}
const xlsx=await SpreadsheetFile.exportXlsx(wb);
const outputPath=path.join(outDir,'半桥LLC与同步整流_完整学习任务与实践计划.xlsx');await xlsx.save(outputPath);
console.log('OUTPUT',outputPath);
