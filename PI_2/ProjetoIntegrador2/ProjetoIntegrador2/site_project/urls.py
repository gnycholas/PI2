from django.urls import path
from .views import *

app_name = 'site_project'

urlpatterns = [
    path('', view_index, name='index'),
    path('get-arduino-data/', get_arduino_data, name='get_arduino_data')
]
