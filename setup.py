import setuptools




with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
     name='mrtutils',
     version='0.1.81',
     author="Jason Berger",
     author_email="JBerger@up-rev.com",
     description="Utilities for MrT",
     long_description=long_description,
     scripts=['mrtutils/mrt-config','mrtutils/mrt-device', 'mrtutils/mrt-ble'],
     long_description_content_type="text/markdown",
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
        'update_notipy'
     ],
     classifiers=[
         "Programming Language :: Python :: 3",
         "License :: OSI Approved :: MIT License",
         "Operating System :: OS Independent",
     ],
 )
