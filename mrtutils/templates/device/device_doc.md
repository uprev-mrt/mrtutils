
# ${obj.name} Device

<ul>
  <li> Generated with <a href="https://github.com/uprev-mrt/mrtutils/wiki/mrt-device">MrT Device Utility</a> </li>
  <li> Bus:  ${obj.bus}</li>
  <li> RegMap: <a href="Regmap.html">Register Map</a>
  %if not obj.datasheet == '':
  <li>Datasheet: <a href="${obj.datasheet}">${obj.datasheet[:24] + (obj.datasheet[24:] and '...')}</a> </li>
  %endif
  %if not obj.digikey_pn == '':
  <li> DigiKey: <a href="https://www.digikey.com/products/en?KeyWords=${obj.digikey_pn}">${obj.digikey_pn}</a></li>
  %endif
  %if "I2C" in obj.bus.upper():
  <li> I2C Address: ${ obj.formatHex(obj.i2c_addr, 1)}</li>
  %endif
</ul>
<hr/>
<h2>Description: </h2>
<p>${obj.desc}</p>

<!--*user-block-description-start*-->

<!--*user-block-description-end*-->
<br/>


<hr class="section">
<h2 class="right"> Register Map</h2>
<hr class="thick">

<table class="fields">
    <tr>
        <th>Name</th>
        <th>Address</th>
        <th>Type</th>
        <th>Access</th>
        <th>Default</th>
        <th>Description</th>
    </tr>
    %for key,reg in obj.regs.items():
    <tr>
        <td><a href="#register_${reg.name.lower()}_detail">${reg.name}</a></td>
        <td>${reg.printAddr()}</td>
        <td>${reg.type}</td>
        <td>${reg.perm.upper()}</td>
        <td>${reg.formatHex(reg.default)}</td>
        <td>${reg.desc}</td>
    </tr>
    %endfor

</table>



<hr class="section">
<h2 class="right"> Registers</h2>
<hr class="thick">



%for key,reg in obj.regs.items():
<div id="register_${reg.name.lower()}_detail" class="packet">
<h2>${reg.name} </h2>
<hr/>
<ul>
    <li class="note">  Address: <b>[${reg.printAddr()}]</b></li>
    %if reg.hasDefault:
    <li class="note">  Default: <b>[${reg.formatHex(reg.default)}]</b></li>
    %endif
</ul>

<p>${reg.desc}</p>
<!--*user-block-${reg.name.lower()}-start*-->

<!--*user-block-${reg.name.lower()}-end*-->
<table class="fields" width="80%">
  <tr>
    <th class="smallCell">bit</th>
    %for i in range(reg.size * 8):
    <th> ${(reg.size*8) -(i+1)}</th>
    %endfor
  </tr>
  <tr>
    <th class="smallCell">Field</th>
   ${reg.printFieldMap()}
  </tr>
  %if reg.hasDefault:
  <tr>
    <th class="smallCell">Default</th>
    %for i in range(reg.size * 8):
    %if reg.default & (1 << ((reg.size * 8) -(i+1))):
      <td class="one" >1</td>
    %else:
      <td class="zero" >0</td>
    %endif
    %endfor
   </tr>
   %endif
</table>

%if reg.hasFlags:
<h2> Flags:</h2>
%for field in reg.fields:
%if field.isFlag:
<b>${field.name}:</b> ${field.desc}<br>
%endif
%endfor
%endif

%if reg.hasFields:
<h2> Fields:</h2>
%for field in reg.fields:
%if not field.isFlag:

<b>${field.name}:</b> ${field.desc}
%if len(field.vals) > 0:
<table>
%for val in field.vals:
<tr><td> ${val.name} </td><td> ${val.formatVal()} </td><td>  ${val.desc}</td></tr>
%endfor
</table>

%endif

%endif
%endfor
%endif

%endfor
