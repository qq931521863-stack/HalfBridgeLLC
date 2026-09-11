"""Generate VS 2022 x64 projects with explicit sources (no ARM libraries)."""
from pathlib import Path
import uuid,html
ROOT=Path(__file__).resolve().parents[1]
GUID=lambda n:'{'+str(uuid.uuid5(uuid.NAMESPACE_URL,'llc-plecs-port/'+n)).upper()+'}'
projects={
 'LLC_Probe':('DynamicLibrary',['adapters/plecs_probe.c']),
 'LLC_Fast':('DynamicLibrary',['adapters/plecs_fast.c','core/llc_control.c','core/llc_fast_task.c','core/llc_timer_image.c']),
 'test_core':('Application',['tests/test_core.c','core/llc_control.c','core/llc_fast_task.c','core/llc_timer_image.c']),
 'test_timer':('Application',['tests/test_timer.c','core/llc_timer_image.c']),
 'test_gate':('Application',['tests/test_gate.c','model/gate/llc_gate_events.c'])}
projects['LLC_Slow']=('DynamicLibrary',['adapters/plecs_slow.c','supervisor/llc_slow_task.c'])
projects['LLC_Combined']=('DynamicLibrary',['adapters/plecs_combined.c','core/llc_control.c','core/llc_fast_task.c','core/llc_timer_image.c','supervisor/llc_slow_task.c'])
projects['test_slow']=('Application',['tests/test_slow.c','supervisor/llc_slow_task.c'])
def generate():
 sln=['Microsoft Visual Studio Solution File, Format Version 12.00','# Visual Studio Version 17']
 for name,(kind,files) in projects.items():
  cfg=''.join(f'<ProjectConfiguration Include="{c}|x64"><Configuration>{c}</Configuration><Platform>x64</Platform></ProjectConfiguration>' for c in ['Debug','Release'])
  groups=''.join(f'''<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='{c}|x64'" Label="Configuration"><ConfigurationType>{kind}</ConfigurationType><UseDebugLibraries>{str(c=='Debug').lower()}</UseDebugLibraries><PlatformToolset>v143</PlatformToolset><CharacterSet>Unicode</CharacterSet></PropertyGroup>''' for c in ['Debug','Release'])
  items=''.join('<ClCompile Include="'+html.escape(f.replace('/','\\'))+'" />' for f in files)
  xml=f'''<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
<ItemGroup Label="ProjectConfigurations">{cfg}</ItemGroup>
<PropertyGroup Label="Globals"><ProjectGuid>{GUID(name)}</ProjectGuid><WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion></PropertyGroup>
<Import Project="$(VCTargetsPath)\\Microsoft.Cpp.Default.props" />{groups}
<Import Project="$(VCTargetsPath)\\Microsoft.Cpp.props" />
<PropertyGroup><OutDir>$(ProjectDir)bin\\x64\\</OutDir><IntDir>$(ProjectDir)build\\$(Configuration)\\{name}\\</IntDir><TargetName>{name}</TargetName></PropertyGroup>
<ItemDefinitionGroup><ClCompile><WarningLevel>Level4</WarningLevel><TreatWarningAsError>true</TreatWarningAsError><CompileAs>CompileAsC</CompileAs><DebugInformationFormat>OldStyle</DebugInformationFormat><AdditionalIncludeDirectories>$(ProjectDir)include;$(ProjectDir)vendor\\plecs;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories><AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions></ClCompile><Link><GenerateDebugInformation>true</GenerateDebugInformation></Link></ItemDefinitionGroup>
<ItemGroup>{items}</ItemGroup><Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" /></Project>
'''
  (ROOT/f'{name}.vcxproj').write_text(xml,encoding='utf-8')
  sln+=['Project("{BC8A1FFA-BEE3-4634-8014-F334798102B3}") = "'+name+'", "'+name+'.vcxproj", "'+GUID(name)+'"','EndProject']
 sln+=['Global','\tGlobalSection(SolutionConfigurationPlatforms) = preSolution','\t\tDebug|x64 = Debug|x64','\t\tRelease|x64 = Release|x64','\tEndGlobalSection','\tGlobalSection(ProjectConfigurationPlatforms) = postSolution']
 for name in projects:
  for c in ['Debug','Release']:
   for opt in ['ActiveCfg','Build.0']: sln+=[f'\t\t{GUID(name)}.{c}|x64.{opt} = {c}|x64']
 sln+=['\tEndGlobalSection','EndGlobal']
 (ROOT/'LLC_Plecs_Port.sln').write_text('\n'.join(sln)+'\n',encoding='utf-8')
if __name__=='__main__':generate(); print('VS projects generated')
