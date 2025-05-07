import google.generativeai as genai

# 使用你的 API_KEY 进行配置
genai.configure(api_key="AIzaSyBPKqN5xUv6pDRxiicZ4_ExR-hJNECX1og")

model = genai.GenerativeModel("gemini-1.5-flash")
response = model.generate_content("你好")
print(response.text)