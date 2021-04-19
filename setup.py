import setuptools




with open("README.rst", "r") as fh:
    long_description = fh.read()

setuptools.setup(
     name='mrtutils',
     version='0.1.87',
     author="Jason Berger",
     author_email="JBerger@up-rev.com",
     description="Utilities for MrT",
     long_description=long_description,
     scripts=['mrtutils/mrt-config','mrtutils/mrt-device', 'mrtutils/mrt-ble', 'mrtutils/mrt-config-gui'],
     long_description_content_type="text/x-rst",
     url="http://www.up-rev.com/",
     packages=setuptools.find_packages(),
     package_data={
     'mrtutils':['templates/*', 'templates/*/*']
     },
     install_requires=[
        'markdown',
        'mako',
        'pyyaml',
        'polypacket',
        'update_notipy',
        'kconfiglib'
     ],
     classifiers=[
         "Programming Language :: Python :: 3",
         "License :: OSI Approved :: MIT License",
         "Operating System :: OS Independent",
     ],
 )
