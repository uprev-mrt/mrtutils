
${obj.name}
%for c in obj.name:
=\
%endfor


- Generated with `MrT Device Utility <https://github.com/uprev-mrt/mrtutils/wiki/mrt-device>`_
- Bus:  ${obj.bus}
- RegMap: `Register Map <Regmap.html>`_
  %if not obj.datasheet == '':
- Datasheet: `${obj.datasheet[:24] + (obj.datasheet[24:] and '...')} <${obj.datasheet}>`_
  %endif
  %if not obj.digikey_pn == '':
- DigiKey: `${obj.digikey_pn} <https://www.digikey.com/products/en?KeyWords=${obj.digikey_pn}>`_
  %endif
  %if "I2C" in obj.bus.upper():
- I2C Address: ${ obj.formatHex(obj.i2c_addr, 1)}
  %endif


Description
-----------

${obj.desc}

.. *user-block-description-start*

.. *user-block-description-end*





Register Map
------------

=================     ================     ================     ================     ================     ================
Name                    Address             Type                  Access              Default               Description
-----------------     ----------------     ----------------     ----------------     ----------------     ----------------
%for key,reg in obj.regs.items():
${t.padAfter(reg.name+"_", 23)}${t.padAfter(reg.printAddr(),21)}${t.padAfter(reg.type,21)}${t.padAfter(reg.perm.upper(),21)}${t.padAfter(reg.formatHex(reg.default),21)}${t.padAfter(reg.desc,21)}
%endfor
=================     ================     ================     ================     ================     ================





Registers
---------




%for key,reg in obj.regs.items():

.. _${reg.name}:

${reg.name}
%for c in reg.name:
=\
%endfor


:Address: **[${reg.printAddr()}]**
    %if reg.hasDefault:
:Default: **[${reg.formatHex(reg.default)}]**
    %endif

${reg.desc}
.. *user-block-${reg.name.lower()}-start*

.. *user-block-${reg.name.lower()}-end*

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