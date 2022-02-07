
${obj.name} Gatt ICD
%for c in obj.name:
=\
%endfor
=========


* Generated with ` MrT BLE Tool <https://github.com/uprev-mrt/mrtutils/wiki/mrt-ble>`_ on ${obj.genTime}
* CRC: ${obj.hash}

----

Description
-----------

${obj.desc}

.. <!--*user-block-description-start*-->
.. <!--*user-block-description-end*-->

----


Table Of Contents
-----------------

.. <!--*user-block-toc-top-start*-->
.. <!--*user-block-toc-top-end*-->

%if len(obj.services) > 0:
* Services
  %for service in obj.services:
    * `${service.name}`_
  %endfor
%endif

.. <!--*user-block-toc-bot-start*-->
.. <!--*user-block-toc-bot-end*-->

%for service in obj.services:

----

${service.name}
%for c in service.name:
~\
%endfor

%if service.isStandard:
    *  *SIG descriptor:* `[${ service.uri}] <${service.url}>`_
    *  *Service UUID:* *[${ "%0.4X" %  service.uuid}]*
%else:
    *  *Service UUID:* *[${ service.uuid}]*
%endif

${service.desc}

.. <!--*user-block-${service.name.lower()}-start*-->

.. <!--*user-block-${service.name.lower()}-end*-->

%if len(service.chars) > 0:

.. table:: 
    :widths: 45,16,24,16,150

    +--------------------------------------------------------+----------------+----------------+----------------+-----------------------------------------------------------------------------------------------------------------------------------------------------+
    |**Characteristic**                                      |**UUID**        |**Type**        |**Perm**        | **Description**                                                                                                                                     |
    +========================================================+================+================+================+=====================================================================================================================================================+
  %for characteristic in service.chars:
    %if characteristic.isStandard:
    |${t.padAfter("**"+characteristic.name+"**",56)+"|"+t.padAfter(characteristic.uuidStr(),16)+"|"+t.padAfter(characteristic.printType(), 16)+"|"+t.padAfter(characteristic.perm,16)+"|"+t.padAfter(characteristic.desc,149) + "|"}                                                                                                                                  
    %else:
    |${t.padAfter("**"+characteristic.name +"**",56)+"|"+t.padAfter(characteristic.uuidStr(),16)+"|"+t.padAfter(characteristic.printType(), 16)+"|"+t.padAfter(characteristic.perm,16)+"|"+t.padAfter(characteristic.desc,149) + "|"}                                                                                                                                  
    %endif
      %if characteristic.isEnum:
        %for idx,val in enumerate(characteristic.vals):
    |${ (" "*56)+"|"+ (" "*16)+"|"+(" "*16)+"|"+(" "*16)+"|"+ t.padAfter(" * {0} : **{1}** - {2}".format(characteristic.valsFormat % idx,val.name,val.desc ),149)+"|"} 
        %endfor
      %endif
      %if characteristic.isMask:
        %for idx,val in enumerate(characteristic.vals):
    |${ (" "*56)+"|"+ (" "*16)+"|"+(" "*16)+"|"+(" "*16)+"|"+ t.padAfter(" * {0} : **{1}** - {2}".format(characteristic.valsFormat % (1 << idx),val.name,val.desc ),149)+"|"} 
        %endfor
      %endif
    +--------------------------------------------------------+----------------+----------------+----------------+-----------------------------------------------------------------------------------------------------------------------------------------------------+
  %endfor



%else:
Zero data chars
~~~~~~~~~~~~~~~
This service type does not contain any data chars 

%endif

%endfor

.. <!--*user-block-bottom-start*-->

.. <!--*user-block-bottom-end*-->


