# Openversa_C_Agent_V2
Openversa C Agent 
<p>
<strong> Summary: </strong> Once downloaded on your platform you must edit ov.c to put your device credentials that were generated from openversa.com and your account. Once the ov.c is edited then you can compile the agent. Once compiled execute the agent to run in the background process. <br><br>The command line execution is noted below.
</p>
<p>
<strong> COMMAND LINE OPENVERSA AGENT EXECUTION</strong> 
<br>
<div>
<p>
<strong>ov upload</strong>  "group name" "local path" "ov filename"<br>
<strong>ov download </strong> "group name" "ov filename" "local path"<br>
<strong>ov monitor</strong>  "group name"<br>
<strong>ov test</strong>  "optional group name"<br>
<strong>ov group</strong>  <br>
<strong>ov file</strong>  "group name"<br>
<br>
</p>
<p>
More Detail:<br>
<br>
<strong> ov upload</strong><br>USAGE: ov upload "group name" "local path" "ov filename"<br>DESCRIPTION: This operation uploads the file in "local path" to the OpenVersa file "ov filename"  in the group <group name>
</p>
<p>
<strong> ov download</strong><br>USAGE: ov download "group name" "ov filenam" "local path"<br>DESCRIPTION: This operation downloads  the OpenVersa file <ov filename>  in the group <group name> to <local path>
</p>
<p>
<strong> ov monitor</strong><br>USAGE: ov monitor "group name"<br>DESCRIPTION: This operation runs in a loop listening for messages sent to OpenVersa group  <group name>. When a message is received with the prefix '!', the subsequent text is treated as a Linux command which is executed on the local machine, and the results are sent back to <group name> as a message.
</p>
<p>
<strong> ov test</strong><br>USAGE: ov test "optional group name"<br>DESCRIPTION: This operation runs a test regression cycle, exercising many parts of the OpenVersa API. The source code is a useful reference for developers to see how the C API can be used in practice.
</p>
<p>
<strong> ov group</strong><br>USAGE: ov group<br>DESCRIPTION: Returns a list of all OpenVersa groups for the designated OpenVersa account
</p>
<p>
<strong> ov file</strong><br>USAGE: ov file "group name"<br>DESCRIPTION: Returns a list of files stored in the OpenVersa group <group name> for the designated OpenVersa account.
</p>
</div>
