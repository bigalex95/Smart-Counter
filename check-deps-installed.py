import ultralytics
import cv2


def main():
    print("--------------------------------")
    print("Hello from smart-counter!")
    print("--------------------------------")
    print("********************************")
    print("--------------------------------")
    print("| ultralytics : Sanity Check ✅ |")
    print("--------------------------------")
    ultralytics.checks()
    print("--------------------------------")
    print("********************************")
    print("--------------------------------")
    print("| opencv-python : Sanity Check ✅ |")
    print("--------------------------------")
    print("OpenCV version: ", cv2.__version__)
    # print(cv2.getBuildInformation())
    print("--------------------------------")
    print("********************************")


if __name__ == "__main__":
    main()
