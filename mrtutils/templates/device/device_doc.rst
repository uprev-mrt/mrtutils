
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

${obj.getRstTable()}





Registers
---------




%for key,reg in obj.regs.items():

----------

.. _${reg.name}:

${reg.name}
%for c in reg.name:
-\
%endfor


:Address: **[${reg.printAddr()}]**
    %if reg.hasDefault:
:Default: **[${reg.formatHex(reg.default)}]**
    %endif

${reg.desc}

.. *user-block-${reg.name.lower()}-start*

.. *user-block-${reg.name.lower()}-end*

${reg.getRstTable()}

%if reg.hasFlags:
Flags
~~~~~

%for field in reg.fields:
%if field.isFlag:
:${field.name}: ${field.desc}
%endif
%endfor
%endif

%if reg.hasFields:
Fields
~~~~~~
%for field in reg.fields:
%if not field.isFlag:

:${field.name}: ${field.desc}
%if len(field.vals) > 0:

${field.getRstTable()}

%endif

%endif
%endfor
%endif

%endfor