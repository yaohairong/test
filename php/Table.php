<?php
class Table
{
    private static $tableList;

    private static $tableArray;

    public static function Load()
    {
        $dir = './json';
        $dh = opendir($dir);
        while (($file = readdir($dh)) !== false)
        {
            if (substr($file, 0, 1) != '.')
            {
                self::$tableList[substr($file, 0, -5)] = json_decode(file_get_contents("./json/$file"));
                self::$tableArray[substr($file, 0, -5)] = json_decode(file_get_contents("./json/$file"), true);
            }
        }
        closedir($dh);
    }

    public static function List($tableName)
    {
        return self::$tableList[$tableName];
    }

    public static function Array($tableName)
    {
        return self::$tableArray[$tableName];
    }

    public static function Get($tableName, $key)
    {
        return self::$tableList[$tableName]->{$key};
    }

    public static function Find($tableName, $arrayKey)
    {
        foreach(self::$tableList[$tableName] as $tableValue)
        {
            $find = true;
            foreach($arrayKey as $key =>$keyValue)
            {
                if ($tableValue->{$key} != $keyValue)
                {
                    $find = false;
                    break;
                }
            }

            if ($find)
            {
                return $tableValue;
            }
        }
    }

    public static function FindArray($tableName, $arrayKey)
    {
        $result = null;
        foreach(self::$tableList[$tableName] as $tableValue)
        {
            $find = true;
            foreach($arrayKey as $key =>$keyValue)
            {
                if ($tableValue->{$key} != $keyValue)
                {
                    $find = false;
                    break;
                }
            }

            if ($find)
            {
                $result[] = $tableValue;
            }
        }
        return $result;
    }

    public static function Test()
    {
        self::Load();

        print_r(self::List('hero'));
        print_r(self::Array('hero'));
        print_r(self::Get('hero', 3));
        print_r(self::Find('hero', ['rare' => 10, 'type' => 2]));
        print_r(self::FindArray('hero', ['rare' => 10, 'type' => 1]));
    }
}
//Table::Test();
?>
