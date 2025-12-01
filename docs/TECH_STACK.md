# 🛠️ Tech Stack & Resources

## 📚 Основные технологии

### 1. Python

**Версия:** 3.9+

**Официальный сайт:** https://www.python.org/

---

### 2. OpenCV (Computer Vision)

**Для чего:** Работа с видео, обработка изображений

**Установка:**

```bash
pip install opencv-python
```

**Документация:** https://docs.opencv.org/

**Примеры использования:**

```python
import cv2

# Открыть видео
cap = cv2.VideoCapture('video.mp4')

# Прочитать кадр
ret, frame = cap.read()

# Изменить размер
frame = cv2.resize(frame, (640, 480))

# Нарисовать прямоугольник
cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
```

**Уроки:**

- https://docs.opencv.org/4.x/d6/d00/tutorial_py_root.html
- https://www.youtube.com/watch?v=oXlwWbU8l2o (FreeCodeCamp)

---

### 3. YOLOv8 (Object Detection)

**Для чего:** Детекция людей на кадрах

**Установка:**

```bash
pip install ultralytics
```

**Официальный сайт:** https://docs.ultralytics.com/

**GitHub:** https://github.com/ultralytics/ultralytics

**Quickstart:**

```python
from ultralytics import YOLO

# Загрузить модель
model = YOLO('yolov8s.pt')

# Запустить детекцию
results = model.predict('image.jpg', conf=0.5)

# Получить bbox
for result in results:
    boxes = result.boxes
    for box in boxes:
        x1, y1, x2, y2 = box.xyxy[0]
        confidence = box.conf[0]
        class_id = box.cls[0]
```

**Туториалы:**

- https://docs.ultralytics.com/modes/predict/
- https://www.youtube.com/watch?v=WgPbbWmnXJ8 (Ultralytics YOLOv8)

**Какую модель выбрать:**

- `yolov8n.pt` - самая быстрая, но менее точная
- `yolov8s.pt` - **рекомендуется** (баланс скорости и точности)
- `yolov8m.pt` - средняя
- `yolov8l.pt` - большая, более точная
- `yolov8x.pt` - огромная, самая точная, но медленная

---

### 4. Object Tracking

#### Вариант A: Встроенный трекер в Ultralytics

**Для чего:** Отслеживание объектов между кадрами

**Документация:** https://docs.ultralytics.com/modes/track/

**Использование:**

```python
from ultralytics import YOLO

model = YOLO('yolov8s.pt')

# Трекинг на видео
results = model.track(
    source='video.mp4',
    tracker='botsort.yaml'  # или bytetrack.yaml
)
```

**Доступные трекеры:**

- `botsort.yaml` - **рекомендуется** (лучшая точность)
- `bytetrack.yaml` - быстрее, но менее точный

#### Вариант B: Supervision (более гибкий)

**Установка:**

```bash
pip install supervision
```

**Сайт:** https://supervision.roboflow.com/

**GitHub:** https://github.com/roboflow/supervision

**Пример:**

```python
import supervision as sv
from ultralytics import YOLO

model = YOLO('yolov8s.pt')
tracker = sv.ByteTrack()

results = model(frame)[0]
detections = sv.Detections.from_ultralytics(results)
detections = tracker.update_with_detections(detections)
```

---

### 5. NumPy

**Для чего:** Работа с массивами (кадры - это numpy arrays)

**Установка:**

```bash
pip install numpy
```

**Документация:** https://numpy.org/doc/

---

### 6. PyYAML

**Для чего:** Чтение конфигурационных файлов

**Установка:**

```bash
pip install pyyaml
```

**Пример:**

```python
import yaml

with open('config.yaml') as f:
    config = yaml.safe_load(f)

threshold = config['detection']['confidence_threshold']
```

---

## 📖 Обучающие материалы

### Базовые концепции

**Computer Vision:**

- https://www.youtube.com/watch?v=01sAkU_NvOY (CS231n Stanford)
- https://www.coursera.org/learn/introduction-computer-vision-watson-opencv

**Object Detection:**

- https://www.youtube.com/watch?v=5e5pjeojznk (YOLO explained)
- https://arxiv.org/abs/2304.00501 (YOLOv8 paper)

**Object Tracking:**

- https://www.youtube.com/watch?v=VVdHB38L0nk (Object Tracking Overview)
- https://arxiv.org/abs/2207.12202 (BoTSORT paper)

---

## 🎓 Похожие проекты (для вдохновения)

### 1. People Counter с YOLOv8

**GitHub:** https://github.com/RizwanMunawar/yolov8-object-tracking

- Простой пример tracking + counting
- Использует Ultralytics встроенный трекер

### 2. Supervision Examples

**GitHub:** https://github.com/roboflow/supervision/tree/develop/examples

- Много примеров детекции и трекинга
- Хорошо документированы

### 3. OpenCV People Counter

**Tutorial:** https://pyimagesearch.com/2018/08/13/opencv-people-counter/

- Классический подход без YOLO
- Хорошо объясняет логику подсчёта

---

## 🔨 Альтернативные подходы

### Без трекинга (простой, но неточный)

Просто считай количество людей на каждом кадре:

```python
count_per_frame = len(detections)
average_count = sum(counts) / len(counts)
```

**Плюсы:** Очень просто
**Минусы:** Не отслеживает вход/выход, только "сколько людей в кадре"

### С линией ROI (Region of Interest)

Считай только людей, которые попали в определённую зону:

```python
if bbox_center_in_roi(bbox, roi):
    count += 1
```

### С heatmap (тепловая карта)

Визуализируй где люди проводят больше всего времени

---

## 💻 Полезные инструменты

### Roboflow (аннотация данных)

https://roboflow.com/

- Если хочешь обучить свою модель
- Удобный интерфейс для разметки

### Netron (визуализация моделей)

https://netron.app/

- Посмотреть архитектуру нейросети

### Weights & Biases (мониторинг экспериментов)

https://wandb.ai/

- Отслеживание метрик обучения

---

## 📝 Рекомендации

### Начни с простого:

1. ✅ Прочитай видео через OpenCV
2. ✅ Запусти YOLO детекцию на одном кадре
3. ✅ Визуализируй bbox
4. ✅ Добавь трекинг (используй готовый из Ultralytics)
5. ✅ Реализуй логику подсчёта

### Не изобретай велосипед:

- Используй готовые модели YOLO (не обучай с нуля)
- Используй встроенные трекеры (BoTSORT работает хорошо)
- Сосредоточься на логике подсчёта - это самая интересная часть!

### Оптимизация (когда базовая версия работает):

- Уменьши resolution видео (640x480 вместо 1920x1080)
- Пропускай кадры (обрабатывай каждый 2-й или 3-й)
- Используй GPU (CUDA) если есть
- Используй меньшую модель (yolov8n вместо yolov8s)

---

Удачи! 🚀
