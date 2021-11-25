from django.shortcuts import render
from django.http import JsonResponse
from .models import Datas
import json


# Create your views here.

def view_index(request):
    return render(request, 'index.html')


# view temporaria
def get_arduino_data(request):
    valor_atual = Datas.objects.all().last()
    arduino_data = {
        'calculodavazao': valor_atual.calculodavazao,
        'diff_hour': valor_atual.diff_hour,
        'hour': valor_atual.calculodavazao,
        'id': valor_atual.id,
    }
    return JsonResponse(arduino_data)
