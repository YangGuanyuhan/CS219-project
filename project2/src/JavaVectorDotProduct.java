import java.util.Scanner;

public class JavaVectorDotProduct {
    // Constants for error codes
    private static final int DOT_PRODUCT_SUCCESS = 0;
    private static final int DOT_PRODUCT_NULL_PTR_ERROR = 1;
    private static final int DOT_PRODUCT_INVALID_SIZE_ERROR = 2;
    
    // Integer dot product
    public static int dotProductInt(int[] vecA, int[] vecB, int size, int[] result) {
        // Error checking
        if (vecA == null || vecB == null || result == null) {
            return DOT_PRODUCT_NULL_PTR_ERROR;
        }
        
        if (size <= 0) {
            return DOT_PRODUCT_INVALID_SIZE_ERROR;
        }
        
        // Calculate dot product
        result[0] = 0;
        for (int i = 0; i < size; i++) {
            result[0] += vecA[i] * vecB[i];
        }
        
        return DOT_PRODUCT_SUCCESS;
    }
    
    // Long dot product
    public static int dotProductLong(long[] vecA, long[] vecB, int size, long[] result) {
        // Error checking
        if (vecA == null || vecB == null || result == null) {
            return DOT_PRODUCT_NULL_PTR_ERROR;
        }
        
        if (size <= 0) {
            return DOT_PRODUCT_INVALID_SIZE_ERROR;
        }
        
        // Calculate dot product
        result[0] = 0;
        for (int i = 0; i < size; i++) {
            result[0] += vecA[i] * vecB[i];
        }
        
        return DOT_PRODUCT_SUCCESS;
    }
    
    // Short dot product
    public static int dotProductShort(short[] vecA, short[] vecB, int size, short[] result) {
        // Error checking
        if (vecA == null || vecB == null || result == null) {
            return DOT_PRODUCT_NULL_PTR_ERROR;
        }
        
        if (size <= 0) {
            return DOT_PRODUCT_INVALID_SIZE_ERROR;
        }
        
        // Calculate dot product
        result[0] = 0;
        for (int i = 0; i < size; i++) {
            result[0] += vecA[i] * vecB[i];
        }
        
        return DOT_PRODUCT_SUCCESS;
    }
    
    // Byte (char) dot product
    public static int dotProductByte(byte[] vecA, byte[] vecB, int size, byte[] result) {
        // Error checking
        if (vecA == null || vecB == null || result == null) {
            return DOT_PRODUCT_NULL_PTR_ERROR;
        }
        
        if (size <= 0) {
            return DOT_PRODUCT_INVALID_SIZE_ERROR;
        }
        
        // Calculate dot product
        result[0] = 0;
        for (int i = 0; i < size; i++) {
            result[0] += vecA[i] * vecB[i];
        }
        
        return DOT_PRODUCT_SUCCESS;
    }
    
    // Float dot product
    public static int dotProductFloat(float[] vecA, float[] vecB, int size, float[] result) {
        // Error checking
        if (vecA == null || vecB == null || result == null) {
            return DOT_PRODUCT_NULL_PTR_ERROR;
        }
        
        if (size <= 0) {
            return DOT_PRODUCT_INVALID_SIZE_ERROR;
        }
        
        // Calculate dot product
        result[0] = 0.0f;
        for (int i = 0; i < size; i++) {
            result[0] += vecA[i] * vecB[i];
        }
        
        return DOT_PRODUCT_SUCCESS;
    }
    
    // Double dot product
    public static int dotProductDouble(double[] vecA, double[] vecB, int size, double[] result) {
        // Error checking
        if (vecA == null || vecB == null || result == null) {
            return DOT_PRODUCT_NULL_PTR_ERROR;
        }
        
        if (size <= 0) {
            return DOT_PRODUCT_INVALID_SIZE_ERROR;
        }
        
        // Calculate dot product
        result[0] = 0.0;
        for (int i = 0; i < size; i++) {
            result[0] += vecA[i] * vecB[i];
        }
        
        return DOT_PRODUCT_SUCCESS;
    }
    
    // Print error message
    public static void printError(int errorCode) {
        switch (errorCode) {
            case DOT_PRODUCT_NULL_PTR_ERROR:
                System.out.println("Error: Null pointer provided");
                break;
            case DOT_PRODUCT_INVALID_SIZE_ERROR:
                System.out.println("Error: Invalid vector size");
                break;
            default:
                System.out.println("Unknown error: " + errorCode);
        }
    }

    // Parse vector from string, handling different data types
    private static Object parseVector(String vectorStr, String dataType, int size) {
        // Remove [ and ] characters
        vectorStr = vectorStr.substring(1, vectorStr.length() - 1);
        String[] elements = vectorStr.split(",");
        
        switch (dataType) {
            case "int":
                int[] intVec = new int[size];
                for (int i = 0; i < size; i++) {
                    intVec[i] = Integer.parseInt(elements[i].trim());
                }
                return intVec;
                
            case "long":
                long[] longVec = new long[size];
                for (int i = 0; i < size; i++) {
                    longVec[i] = Long.parseLong(elements[i].trim());
                }
                return longVec;
                
            case "short":
                short[] shortVec = new short[size];
                for (int i = 0; i < size; i++) {
                    shortVec[i] = Short.parseShort(elements[i].trim());
                }
                return shortVec;
                
            case "char":
                byte[] byteVec = new byte[size];
                for (int i = 0; i < size; i++) {
                    byteVec[i] = Byte.parseByte(elements[i].trim());
                }
                return byteVec;
                
            case "float":
                float[] floatVec = new float[size];
                for (int i = 0; i < size; i++) {
                    floatVec[i] = Float.parseFloat(elements[i].trim());
                }
                return floatVec;
                
            case "double":
                double[] doubleVec = new double[size];
                for (int i = 0; i < size; i++) {
                    doubleVec[i] = Double.parseDouble(elements[i].trim());
                }
                return doubleVec;
                
            default:
                return null;
        }
    }
    
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        
        // Start timing
        long startTime = System.nanoTime();
        
        // Read number of test cases
        int n = scanner.nextInt();
        
        for (int testCase = 0; testCase < n; testCase++) {
            // Read data type and size
            String dataType = scanner.next();
            int size = scanner.nextInt();
            
            // Check if size is valid
            if (size <= 0) {
                System.out.println("Error: Invalid vector size");
                continue;
            }
            
            // Read vectors - assuming format [a,b,c] [d,e,f]
            String vecAStr = scanner.next(); // Read first vector
            String vecBStr = scanner.next(); // Read second vector
            
            // Process based on data type
            switch (dataType) {
                case "int":
                    int[] vecA = (int[]) parseVector(vecAStr, dataType, size);
                    int[] vecB = (int[]) parseVector(vecBStr, dataType, size);
                    int[] result = new int[1];
                    
                    int status = dotProductInt(vecA, vecB, size, result);
                    
                    if (status == DOT_PRODUCT_SUCCESS) {
                        System.out.println("Int dot product result: " + result[0]);
                    } else {
                        printError(status);
                    }
                    break;
                    
                case "long":
                    long[] vecALong = (long[]) parseVector(vecAStr, dataType, size);
                    long[] vecBLong = (long[]) parseVector(vecBStr, dataType, size);
                    long[] resultLong = new long[1];
                    
                    status = dotProductLong(vecALong, vecBLong, size, resultLong);
                    
                    if (status == DOT_PRODUCT_SUCCESS) {
                        System.out.println("Long dot product result: " + resultLong[0]);
                    } else {
                        printError(status);
                    }
                    break;
                    
                case "short":
                    short[] vecAShort = (short[]) parseVector(vecAStr, dataType, size);
                    short[] vecBShort = (short[]) parseVector(vecBStr, dataType, size);
                    short[] resultShort = new short[1];
                    
                    status = dotProductShort(vecAShort, vecBShort, size, resultShort);
                    
                    if (status == DOT_PRODUCT_SUCCESS) {
                        System.out.println("Short dot product result: " + resultShort[0]);
                    } else {
                        printError(status);
                    }
                    break;
                    
                case "char":
                    byte[] vecAByte = (byte[]) parseVector(vecAStr, dataType, size);
                    byte[] vecBByte = (byte[]) parseVector(vecBStr, dataType, size);
                    byte[] resultByte = new byte[1];
                    
                    status = dotProductByte(vecAByte, vecBByte, size, resultByte);
                    
                    if (status == DOT_PRODUCT_SUCCESS) {
                        System.out.println("Char dot product result: " + (int)resultByte[0]);
                    } else {
                        printError(status);
                    }
                    break;
                    
                case "float":
                    float[] vecAFloat = (float[]) parseVector(vecAStr, dataType, size);
                    float[] vecBFloat = (float[]) parseVector(vecBStr, dataType, size);
                    float[] resultFloat = new float[1];
                    
                    status = dotProductFloat(vecAFloat, vecBFloat, size, resultFloat);
                    
                    if (status == DOT_PRODUCT_SUCCESS) {
                        System.out.println("Float dot product result: " + resultFloat[0]);
                    } else {
                        printError(status);
                    }
                    break;
                    
                case "double":
                    double[] vecADouble = (double[]) parseVector(vecAStr, dataType, size);
                    double[] vecBDouble = (double[]) parseVector(vecBStr, dataType, size);
                    double[] resultDouble = new double[1];
                    
                    status = dotProductDouble(vecADouble, vecBDouble, size, resultDouble);
                    
                    if (status == DOT_PRODUCT_SUCCESS) {
                        System.out.println("Double dot product result: " + resultDouble[0]);
                    } else {
                        printError(status);
                    }
                    break;
                    
                default:
                    System.out.println("Error: Unsupported data type: " + dataType);
            }
        }
        
        // End timing and calculate elapsed time
        long endTime = System.nanoTime();
        long elapsedTimeUs = (endTime - startTime) / 1000; // Convert from ns to µs
        
        System.out.println("程序运行时间: " + elapsedTimeUs + " 微秒");
        
        scanner.close();
    }
}