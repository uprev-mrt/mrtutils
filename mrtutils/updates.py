from update_notipy import update_notify
import pkg_resources

__version__ = pkg_resources.get_distribution("mrtutils").version

def updateCheck():
    update_notify('mrtutils', __version__).notify()

#    ┌───────────────────────────────────────────┐
#    │                                           │
#    │   Update available 0.1.0 → 0.1.2          │
#    │   Run pip install -U pkg-info to update   │
#    │                                           │
#    └───────────────────────────────────────────┘
