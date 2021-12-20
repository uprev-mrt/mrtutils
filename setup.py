import setuptools




with open("README.rst", "r") as fh:
    long_description = fh.read()

setuptools.setup(
     name='mrtutils',
     version='0.2.10',
     author="Jason Berger",
     author_email="JBerger@up-rev.com",
     description="Utilities for MrT",
     long_description=long_description,
     scripts=['mrtutils/mrt-config','mrtutils/mrt-device', 'mrtutils/mrt-ble', 'mrtutils/mrt-config-gui','mrtutils/mrt-doc', 'mrtutils/mrt-version','mrtutils/mrt-gen'],
     long_description_content_type="text/x-rst",
     url="https://mrt.readthedocs.io/en/latest/index.html",
     packages=setuptools.find_packages(),
     package_data={
     'mrtutils':['templates/*', 'templates/*/*','templates/*/*/*']
     },
     install_requires=[
        'markdown',
        'mako',
        'pyyaml',
        'polypacket',
        'update_notipy',
        'kconfiglib',
        'gitpython'
     ],
     classifiers=[
         "Programming Language :: Python :: 3",
         "License :: OSI Approved :: MIT License",
         "Operating System :: OS Independent",
     ],
 )
